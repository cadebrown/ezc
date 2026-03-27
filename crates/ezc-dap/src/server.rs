/// Main DAP server loop.
///
/// `DapServer` owns the I/O streams and the debug session. It reads requests
/// from the client (VS Code), dispatches them to the session, and writes
/// responses and events back.
///
/// The server is intentionally synchronous — EZC programs are single-threaded
/// and all stepping operations complete before the next message is read.
use std::io::{BufRead, Write};
use std::path::PathBuf;

use serde_json::{json, Value};
use tracing::{debug, warn};

use crate::codec::{read_message, write_message};
use crate::protocol::{
    self, parse_args, DapBreakpointResult, DapScope, DapSource, DapStackFrame, DapThread,
    DapVariable, EvaluateArgs, LaunchArgs, RawRequest, ScopesArgs, SetBreakpointsArgs,
    SetExceptionBreakpointsArgs, StackTraceArgs, VariablesArgs,
};
use ezc::eval::Engine;
use ezc::stepper::{Breakpoint, ScopeVariable, Stepper, StepperState, StopReason};

const THREAD_ID: i64 = 1;

// ── Variable reference constants ─────────────────────────────────────────
const VAR_REF_BINDINGS: usize = 1;
const VAR_REF_STACK: usize = 2;
const VAR_REF_DYNAMIC_START: usize = 3;

// ── Session ───────────────────────────────────────────────────────────────

/// Active debug session state.
struct Session {
    stepper: Stepper,
    source_path: String,
    /// Flat store of variable lists indexed by variablesReference.
    /// Index 0 is unused (0 means "no children" in DAP).
    /// Index 1 = bindings scope, 2 = stack scope, 3+ = dynamic children.
    var_store: Vec<Vec<DapVariable>>,
    /// Whether to break on uncaught exceptions.
    break_on_exceptions: bool,
}

impl Session {
    fn new(stepper: Stepper, source_path: String) -> Self {
        Session {
            stepper,
            source_path,
            var_store: vec![vec![]],
            break_on_exceptions: false,
        }
    }

    /// Rebuild the variable store after each pause so VS Code can query it.
    fn rebuild_var_store(&mut self) {
        let scope_vars = self.stepper.scope_variables();
        let stack_vars = self.stepper.stack_variables();

        // Reset store keeping index 0 as unused sentinel
        self.var_store.truncate(1);

        // Index 1: bindings scope
        let binding_dap = build_dap_variables(&scope_vars, &mut self.var_store);
        self.var_store.push(binding_dap); // now at index 1

        // Index 2: stack scope
        let stack_dap = build_dap_variables(&stack_vars, &mut self.var_store);
        self.var_store.push(stack_dap); // now at index 2

        debug!(
            bindings = self
                .var_store
                .get(VAR_REF_BINDINGS)
                .map(|v| v.len())
                .unwrap_or(0),
            stack = self
                .var_store
                .get(VAR_REF_STACK)
                .map(|v| v.len())
                .unwrap_or(0),
            "var store rebuilt"
        );
    }
}

/// Recursively convert `ScopeVariable`s into DAP variables, allocating
/// child references in the store as needed.
fn build_dap_variables(
    items: &[ScopeVariable],
    store: &mut Vec<Vec<DapVariable>>,
) -> Vec<DapVariable> {
    items
        .iter()
        .map(|sv| {
            let child_ref = if sv.children.is_empty() {
                0
            } else {
                let idx = store.len() as i64;
                store.push(vec![]); // placeholder — filled after recursion
                let children = build_dap_variables(&sv.children, store);
                store[idx as usize] = children;
                idx
            };
            DapVariable {
                name: sv.name.clone(),
                value: sv.value.clone(),
                var_type: sv.type_name.clone(),
                variables_reference: child_ref,
                named_variables: None,
                indexed_variables: if sv.children.is_empty() {
                    None
                } else {
                    Some(sv.children.len() as i64)
                },
                presentation_hint: None,
            }
        })
        .collect()
}

// ── DapServer ─────────────────────────────────────────────────────────────

pub struct DapServer<R: BufRead, W: Write> {
    reader: R,
    writer: W,
    /// Monotonically increasing sequence number for outgoing messages.
    seq: i64,
    /// Present after a successful `launch` request.
    session: Option<Session>,
    /// Breakpoints registered before `launch` (buffered until we have a path).
    pending_breakpoints: Vec<(Option<String>, Vec<Breakpoint>)>,
    /// Whether the `initialized` event has been sent.
    initialized: bool,
    /// Whether to stop on entry (set from `launch` args, applied after
    /// `configurationDone`).
    stop_on_entry: bool,
    /// Pending launch info (stored between `launch` and `configurationDone`).
    pending_launch: Option<LaunchArgs>,
}

impl<R: BufRead, W: Write> DapServer<R, W> {
    pub fn new(reader: R, writer: W) -> Self {
        DapServer {
            reader,
            writer,
            seq: 0,
            session: None,
            pending_breakpoints: Vec::new(),
            initialized: false,
            stop_on_entry: true,
            pending_launch: None,
        }
    }

    /// Main loop: read → dispatch → write until the client disconnects.
    pub fn run(&mut self) -> Result<(), Box<dyn std::error::Error>> {
        loop {
            let msg = match read_message(&mut self.reader) {
                Ok(m) => m,
                Err(e) if e.kind() == std::io::ErrorKind::UnexpectedEof => {
                    debug!("client disconnected");
                    return Ok(());
                }
                Err(e) => return Err(e.into()),
            };

            debug!(msg = %msg, "← received");

            let msg_type = msg.get("type").and_then(|v| v.as_str()).unwrap_or("");
            if msg_type != "request" {
                continue; // only handle requests
            }

            let req: RawRequest = serde_json::from_value(msg.clone())?;
            let should_exit = self.handle_request(&req)?;
            if should_exit {
                return Ok(());
            }
        }
    }

    // ── Request dispatch ─────────────────────────────────────────────────

    fn handle_request(&mut self, req: &RawRequest) -> Result<bool, Box<dyn std::error::Error>> {
        debug!(command = %req.command, seq = req.seq, "handling request");
        match req.command.as_str() {
            "initialize" => self.on_initialize(req)?,
            "configurationDone" => self.on_configuration_done(req)?,
            "launch" => self.on_launch(req)?,
            "disconnect" => {
                self.on_disconnect(req)?;
                return Ok(true);
            }
            "terminate" => {
                self.on_terminate(req)?;
                return Ok(true);
            }
            "setBreakpoints" => self.on_set_breakpoints(req)?,
            "setExceptionBreakpoints" => self.on_set_exception_breakpoints(req)?,
            "setFunctionBreakpoints" => self.ack(req)?,
            "threads" => self.on_threads(req)?,
            "stackTrace" => self.on_stack_trace(req)?,
            "scopes" => self.on_scopes(req)?,
            "variables" => self.on_variables(req)?,
            "evaluate" => self.on_evaluate(req)?,
            "continue" => self.on_continue(req)?,
            "next" => self.on_next(req)?,
            "stepIn" => self.on_step_in(req)?,
            "stepOut" => self.on_step_out(req)?,
            "pause" => self.on_pause(req)?,
            "source" => self.on_source(req)?,
            other => {
                warn!(command = other, "unknown command");
                self.send_error(req, &format!("unknown command: {other}"))?;
            }
        }
        Ok(false)
    }

    // ── Handlers ─────────────────────────────────────────────────────────

    fn on_initialize(&mut self, req: &RawRequest) -> Result<(), Box<dyn std::error::Error>> {
        let body = protocol::capabilities();
        self.send_response(req, body)?;
        // The `initialized` event tells the client it can send breakpoints
        self.emit_event("initialized", json!({}))?;
        self.initialized = true;
        Ok(())
    }

    fn on_set_breakpoints(&mut self, req: &RawRequest) -> Result<(), Box<dyn std::error::Error>> {
        let args: SetBreakpointsArgs = match parse_args(req) {
            Some(a) => a,
            None => {
                self.send_error(req, "invalid setBreakpoints arguments")?;
                return Ok(());
            }
        };

        let src_path = args.source.path.clone();
        let bps: Vec<Breakpoint> = args
            .breakpoints
            .unwrap_or_default()
            .into_iter()
            .map(|sb| Breakpoint {
                line: sb.line,
                condition: sb.condition,
                log_message: sb.log_message,
            })
            .collect();

        // Build the response breakpoints (all verified — we have the source)
        let result_bps: Vec<DapBreakpointResult> = bps
            .iter()
            .map(|bp| DapBreakpointResult {
                verified: true,
                message: None,
                line: Some(bp.line as i64),
                source: src_path.as_ref().map(|p| DapSource {
                    name: std::path::Path::new(p)
                        .file_name()
                        .and_then(|n| n.to_str())
                        .map(String::from),
                    path: Some(p.clone()),
                    source_reference: None,
                }),
            })
            .collect();

        // Apply to running session or buffer for later
        if let Some(session) = &mut self.session {
            // Only apply if the path matches our source
            if src_path.as_deref() == Some(session.source_path.as_str()) {
                session.stepper.set_breakpoints(bps);
            }
        } else {
            self.pending_breakpoints.push((src_path, bps));
        }

        self.send_response(req, json!({ "breakpoints": result_bps }))?;
        Ok(())
    }

    fn on_set_exception_breakpoints(
        &mut self,
        req: &RawRequest,
    ) -> Result<(), Box<dyn std::error::Error>> {
        if let Some(args) = parse_args::<SetExceptionBreakpointsArgs>(req) {
            if let Some(session) = &mut self.session {
                session.break_on_exceptions = args.filters.iter().any(|f| f == "all");
            } else {
                // Store for when session starts
                let _ = args; // will apply in on_launch
            }
        }
        self.send_response(req, json!({ "breakpoints": [] }))?;
        Ok(())
    }

    fn on_launch(&mut self, req: &RawRequest) -> Result<(), Box<dyn std::error::Error>> {
        let args: LaunchArgs = match parse_args(req) {
            Some(a) => a,
            None => {
                self.send_error(req, "invalid launch arguments (missing 'program'?)")?;
                return Ok(());
            }
        };
        self.stop_on_entry = args.stop_on_entry;
        self.pending_launch = Some(args);
        self.send_response(req, json!({}))?;
        Ok(())
    }

    fn on_configuration_done(
        &mut self,
        req: &RawRequest,
    ) -> Result<(), Box<dyn std::error::Error>> {
        self.ack(req)?;
        if let Some(launch_args) = self.pending_launch.take() {
            self.start_session(launch_args)?;
        }
        Ok(())
    }

    fn start_session(&mut self, args: LaunchArgs) -> Result<(), Box<dyn std::error::Error>> {
        let path = PathBuf::from(&args.program);
        let source = std::fs::read_to_string(&path)
            .map_err(|e| format!("cannot read '{}': {e}", args.program))?;

        let program = ezc::lex_and_parse(&source).map_err(|e| format!("{e}"))?;

        let engine = Engine::new();
        let mut stepper = Stepper::new(engine, program, source.clone(), args.program.clone());

        // Apply pending breakpoints that match this source
        for (src_path, bps) in self.pending_breakpoints.drain(..) {
            if src_path.as_deref() == Some(&args.program) {
                stepper.set_breakpoints(bps);
            }
        }

        let mut session = Session::new(stepper, args.program.clone());
        session.rebuild_var_store();
        self.session = Some(session);

        if args.stop_on_entry && !args.no_debug {
            self.emit_stopped(StopReason::Entry)?;
        } else {
            // Run until first breakpoint or end
            self.do_continue()?;
        }
        Ok(())
    }

    fn on_threads(&mut self, req: &RawRequest) -> Result<(), Box<dyn std::error::Error>> {
        let threads = vec![DapThread {
            id: THREAD_ID,
            name: "main".into(),
        }];
        self.send_response(req, json!({ "threads": threads }))?;
        Ok(())
    }

    fn on_stack_trace(&mut self, req: &RawRequest) -> Result<(), Box<dyn std::error::Error>> {
        let args: StackTraceArgs = parse_args(req).unwrap_or(StackTraceArgs {
            thread_id: THREAD_ID,
            start_frame: None,
            levels: None,
        });

        let frames: Vec<DapStackFrame> = if let Some(session) = &self.session {
            let all_frames = session.stepper.stack_frames();
            let start = args.start_frame.unwrap_or(0);
            let levels = args.levels.unwrap_or(all_frames.len());
            all_frames
                .iter()
                .skip(start)
                .take(levels)
                .map(|fi| DapStackFrame {
                    id: fi.id as i64,
                    name: fi.name.clone(),
                    source: Some(DapSource {
                        name: std::path::Path::new(&fi.source_path)
                            .file_name()
                            .and_then(|n| n.to_str())
                            .map(String::from),
                        path: Some(fi.source_path.clone()),
                        source_reference: None,
                    }),
                    line: fi.line as i64,
                    column: fi.column as i64,
                    end_line: None,
                    end_column: None,
                    presentation_hint: Some("normal".into()),
                })
                .collect()
        } else {
            vec![]
        };

        let total = frames.len();
        self.send_response(req, json!({ "stackFrames": frames, "totalFrames": total }))?;
        Ok(())
    }

    fn on_scopes(&mut self, req: &RawRequest) -> Result<(), Box<dyn std::error::Error>> {
        let _args: ScopesArgs = match parse_args(req) {
            Some(a) => a,
            None => {
                self.send_error(req, "invalid scopes args")?;
                return Ok(());
            }
        };

        let (binding_count, stack_count) = if let Some(session) = &self.session {
            let bc = session
                .var_store
                .get(VAR_REF_BINDINGS)
                .map(|v| v.len())
                .unwrap_or(0);
            let sc = session
                .var_store
                .get(VAR_REF_STACK)
                .map(|v| v.len())
                .unwrap_or(0);
            (bc, sc)
        } else {
            (0, 0)
        };

        let scopes = vec![
            DapScope {
                name: "Variables".into(),
                presentation_hint: Some("locals".into()),
                variables_reference: VAR_REF_BINDINGS as i64,
                named_variables: Some(binding_count as i64),
                indexed_variables: None,
                expensive: false,
                source: None,
                line: None,
                column: None,
                end_line: None,
                end_column: None,
            },
            DapScope {
                name: "Value Stack".into(),
                presentation_hint: Some("registers".into()),
                variables_reference: VAR_REF_STACK as i64,
                named_variables: None,
                indexed_variables: Some(stack_count as i64),
                expensive: false,
                source: None,
                line: None,
                column: None,
                end_line: None,
                end_column: None,
            },
        ];

        self.send_response(req, json!({ "scopes": scopes }))?;
        Ok(())
    }

    fn on_variables(&mut self, req: &RawRequest) -> Result<(), Box<dyn std::error::Error>> {
        let args: VariablesArgs = match parse_args(req) {
            Some(a) => a,
            None => {
                self.send_error(req, "invalid variables args")?;
                return Ok(());
            }
        };

        let vars: Vec<&DapVariable> = if let Some(session) = &self.session {
            let idx = args.variables_reference;
            session
                .var_store
                .get(idx)
                .map(|v| v.iter().collect())
                .unwrap_or_default()
        } else {
            vec![]
        };

        // Apply start/count pagination if provided
        let start = args.start.unwrap_or(0);
        let count = args.count.unwrap_or(vars.len());
        let page: Vec<&DapVariable> = vars.into_iter().skip(start).take(count).collect();

        self.send_response(req, json!({ "variables": page }))?;
        Ok(())
    }

    fn on_evaluate(&mut self, req: &RawRequest) -> Result<(), Box<dyn std::error::Error>> {
        let args: EvaluateArgs = match parse_args(req) {
            Some(a) => a,
            None => {
                self.send_error(req, "invalid evaluate args")?;
                return Ok(());
            }
        };

        let result = if let Some(session) = &mut self.session {
            // Evaluate the expression in a snapshot of the current engine state
            // (save/restore stack so evaluation is side-effect-free for hovers,
            // but allow mutation for the "repl" context).
            let is_hover = args.context.as_deref() == Some("hover");
            let saved = if is_hover {
                Some(session.stepper.engine.clone_raw_stack())
            } else {
                None
            };

            let eval_result = ezc::eval_line(&mut session.stepper.engine, &args.expression);

            if let Some(saved_stack) = saved {
                session.stepper.engine.set_stack(saved_stack);
            }

            match eval_result {
                Ok(()) => {
                    // Return the top of the stack as the result
                    let top = session.stepper.engine.stack().last().map(|v| v.to_string());
                    Ok(top.unwrap_or_else(|| "<empty>".into()))
                }
                Err(e) => Err(format!("{e}")),
            }
        } else {
            Err("no active debug session".into())
        };

        match result {
            Ok(value) => {
                self.send_response(
                    req,
                    json!({
                        "result": value,
                        "variablesReference": 0,
                        "type": "value",
                    }),
                )?;
            }
            Err(msg) => {
                self.send_error(req, &msg)?;
            }
        }
        Ok(())
    }

    fn on_continue(&mut self, req: &RawRequest) -> Result<(), Box<dyn std::error::Error>> {
        self.send_response(req, json!({ "allThreadsContinued": true }))?;
        self.do_continue()?;
        Ok(())
    }

    fn on_next(&mut self, req: &RawRequest) -> Result<(), Box<dyn std::error::Error>> {
        self.ack(req)?;
        if let Some(session) = &mut self.session {
            let state = session.stepper.step_over();
            self.emit_output_events()?;
            self.handle_stepper_state(state)?;
        }
        Ok(())
    }

    fn on_step_in(&mut self, req: &RawRequest) -> Result<(), Box<dyn std::error::Error>> {
        self.ack(req)?;
        if let Some(session) = &mut self.session {
            let state = session.stepper.step_in();
            self.emit_output_events()?;
            self.handle_stepper_state(state)?;
        }
        Ok(())
    }

    fn on_step_out(&mut self, req: &RawRequest) -> Result<(), Box<dyn std::error::Error>> {
        self.ack(req)?;
        if let Some(session) = &mut self.session {
            let state = session.stepper.step_out();
            self.emit_output_events()?;
            self.handle_stepper_state(state)?;
        }
        Ok(())
    }

    fn on_pause(&mut self, req: &RawRequest) -> Result<(), Box<dyn std::error::Error>> {
        // EZC is synchronous — pause is a no-op since we're already paused
        // between requests.
        self.ack(req)?;
        Ok(())
    }

    fn on_source(&mut self, req: &RawRequest) -> Result<(), Box<dyn std::error::Error>> {
        let source_text = self
            .session
            .as_ref()
            .map(|s| s.stepper.source.clone())
            .unwrap_or_default();
        self.send_response(req, json!({ "content": source_text }))?;
        Ok(())
    }

    fn on_disconnect(&mut self, req: &RawRequest) -> Result<(), Box<dyn std::error::Error>> {
        self.ack(req)?;
        self.session = None;
        Ok(())
    }

    fn on_terminate(&mut self, req: &RawRequest) -> Result<(), Box<dyn std::error::Error>> {
        self.ack(req)?;
        self.emit_event("terminated", json!({}))?;
        self.session = None;
        Ok(())
    }

    // ── Stepping helpers ─────────────────────────────────────────────────

    fn do_continue(&mut self) -> Result<(), Box<dyn std::error::Error>> {
        let state = if let Some(session) = &mut self.session {
            session.stepper.continue_execution()
        } else {
            return Ok(());
        };
        self.emit_output_events()?;
        self.handle_stepper_state(state)
    }

    fn handle_stepper_state(
        &mut self,
        state: StepperState,
    ) -> Result<(), Box<dyn std::error::Error>> {
        match state {
            StepperState::Paused(reason) => {
                if let Some(session) = &mut self.session {
                    session.rebuild_var_store();
                }
                self.emit_stopped(reason)?;
            }
            StepperState::Terminated => {
                self.emit_event("terminated", json!({}))?;
                self.emit_event("exited", json!({ "exitCode": 0 }))?;
                self.session = None;
            }
            StepperState::Errored(e) => {
                let msg = format!("{}", e.kind);
                // Emit the error to the debug console
                self.emit_event(
                    "output",
                    json!({
                        "category": "stderr",
                        "output": format!("error: {msg}\n"),
                    }),
                )?;
                if let Some(session) = &self.session {
                    if session.break_on_exceptions {
                        if let Some(sess) = &mut self.session {
                            sess.rebuild_var_store();
                        }
                        self.emit_stopped(StopReason::Exception)?;
                        return Ok(());
                    }
                }
                self.emit_event("terminated", json!({}))?;
                self.emit_event("exited", json!({ "exitCode": 1 }))?;
                self.session = None;
            }
        }
        Ok(())
    }

    fn emit_stopped(&mut self, reason: StopReason) -> Result<(), Box<dyn std::error::Error>> {
        let reason_str = reason.dap_reason();
        let description = match &reason {
            StopReason::Step => "Paused on step".into(),
            StopReason::Breakpoint(line) => format!("Breakpoint hit at line {line}"),
            StopReason::Entry => "Paused at program start".into(),
            StopReason::Exception => "Paused on exception".into(),
        };
        self.emit_event(
            "stopped",
            json!({
                "reason": reason_str,
                "description": description,
                "threadId": THREAD_ID,
                "allThreadsStopped": true,
            }),
        )
    }

    fn emit_output_events(&mut self) -> Result<(), Box<dyn std::error::Error>> {
        let messages: Vec<String> = if let Some(session) = &mut self.session {
            session.stepper.drain_pending_output()
        } else {
            vec![]
        };
        for msg in messages {
            self.emit_event(
                "output",
                json!({
                    "category": "console",
                    "output": format!("{msg}\n"),
                }),
            )?;
        }
        Ok(())
    }

    // ── I/O helpers ───────────────────────────────────────────────────────

    fn send_response(
        &mut self,
        req: &RawRequest,
        body: Value,
    ) -> Result<(), Box<dyn std::error::Error>> {
        let msg = protocol::success_response(&mut self.seq, req.seq, &req.command, body);
        debug!(msg = %msg, "→ sending response");
        write_message(&mut self.writer, &msg)?;
        Ok(())
    }

    fn send_error(
        &mut self,
        req: &RawRequest,
        message: &str,
    ) -> Result<(), Box<dyn std::error::Error>> {
        let msg = protocol::error_response(&mut self.seq, req.seq, &req.command, message);
        debug!(msg = %msg, "→ sending error");
        write_message(&mut self.writer, &msg)?;
        Ok(())
    }

    fn emit_event(&mut self, event: &str, body: Value) -> Result<(), Box<dyn std::error::Error>> {
        let msg = protocol::event(&mut self.seq, event, body);
        debug!(msg = %msg, "→ emitting event");
        write_message(&mut self.writer, &msg)?;
        Ok(())
    }

    /// Send a success response with an empty body.
    fn ack(&mut self, req: &RawRequest) -> Result<(), Box<dyn std::error::Error>> {
        self.send_response(req, json!({}))
    }
}
