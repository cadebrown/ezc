/// Step-wise debugger for EZC programs.
///
/// Wraps an `Engine` with an explicit call stack so the debugger can pause,
/// inspect, and resume execution one expression at a time. This is the core
/// of the Debug Adapter Protocol (DAP) backend.
///
/// # Architecture
///
/// The `Stepper` maintains a `Vec<StepFrame>` parallel to the call stack:
/// - Frame 0 (bottom) is the top-level program.
/// - Higher frames are block invocations that were stepped into via `step_in`.
///
/// `step_over` executes the current expression atomically (including any
/// sub-block calls inside it) and advances the program counter.
///
/// `step_in` intercepts `Execute` (`!`) when the top of the value stack is a
/// `Block`: instead of running the block, it pops it and pushes a new frame.
///
/// `step_out` runs to the end of the current frame, then pauses in the parent.
use std::collections::HashMap;

use crate::ast::{Expr, Spanned};
use crate::error::EvalError;
use crate::eval::Engine;
use crate::line_index::LineIndex;
use crate::types::Value;

// ── Frame ─────────────────────────────────────────────────────────────────

/// A single frame on the debugger call stack.
pub struct StepFrame {
    /// Human-readable name shown in the VS Code call stack panel.
    pub name: String,
    /// Absolute path of the source file this frame belongs to.
    pub source_path: String,
    /// The expressions in this frame.
    pub body: Vec<Spanned<Expr>>,
    /// Index of the *next* expression to execute.
    pub pc: usize,
}

impl StepFrame {
    /// Returns the span of the expression *about to be* executed, or `None`
    /// if the frame has nothing left to execute.
    pub fn current_span(&self) -> Option<std::ops::Range<usize>> {
        self.body.get(self.pc).map(|(_, span)| span.clone())
    }

    /// Returns `true` if there are no more expressions to execute.
    pub fn is_exhausted(&self) -> bool {
        self.pc >= self.body.len()
    }
}

// ── Stop reasons ─────────────────────────────────────────────────────────

/// Why the debugger is paused.
#[derive(Debug, Clone)]
pub enum StopReason {
    /// Single-step completed (`next`, `stepIn`, or `stepOut`).
    Step,
    /// A breakpoint was hit (1-based line number).
    Breakpoint(usize),
    /// Paused before executing the first instruction (`stopOnEntry`).
    Entry,
    /// An evaluation error occurred.
    Exception,
}

impl StopReason {
    /// The string expected by the DAP `stopped` event `reason` field.
    pub fn dap_reason(&self) -> &'static str {
        match self {
            StopReason::Step => "step",
            StopReason::Breakpoint(_) => "breakpoint",
            StopReason::Entry => "entry",
            StopReason::Exception => "exception",
        }
    }
}

/// Outcome of a stepping operation.
#[derive(Debug)]
pub enum StepperState {
    /// The debugger is paused and ready for inspection.
    Paused(StopReason),
    /// The program finished without errors.
    Terminated,
    /// The program raised a runtime error.
    Errored(EvalError),
}

// ── Breakpoints ───────────────────────────────────────────────────────────

/// A source breakpoint — may be conditional or a logpoint.
#[derive(Debug, Clone)]
pub struct Breakpoint {
    /// 1-based line number.
    pub line: usize,
    /// Optional EZC condition expression. Must leave a truthy value on the
    /// stack to trigger a halt. Evaluated in a snapshot of engine state so
    /// the condition cannot corrupt the running program.
    pub condition: Option<String>,
    /// Logpoint message (no halt — emits to the debug console instead).
    pub log_message: Option<String>,
}

// ── Inspection types ─────────────────────────────────────────────────────

/// A named variable in a scope or the value stack.
#[derive(Debug, Clone)]
pub struct ScopeVariable {
    pub name: String,
    pub value: String,
    pub type_name: String,
    /// For `Value::List`: the indexed child items (recursively).
    pub children: Vec<ScopeVariable>,
}

/// Information about a call stack frame (for the DAP `stackTrace` response).
#[derive(Debug, Clone)]
pub struct FrameInfo {
    /// Stable ID for this pause event.
    pub id: usize,
    pub name: String,
    pub source_path: String,
    /// 1-based line and column.
    pub line: usize,
    pub column: usize,
}

// ── Stepper ───────────────────────────────────────────────────────────────

/// The EZC step-wise debugger.
pub struct Stepper {
    /// The underlying execution engine.
    pub engine: Engine,
    /// Call stack — index 0 is the top-level program, last is the current frame.
    pub frames: Vec<StepFrame>,
    /// Absolute path of the source file being debugged.
    pub source_path: String,
    /// Raw source text (kept for error messages and span lookups).
    pub source: String,
    /// Maps byte offsets to line/column positions.
    pub line_index: LineIndex,
    /// Active breakpoints keyed by 1-based line number.
    pub breakpoints: HashMap<usize, Breakpoint>,
    /// Log messages queued by logpoint breakpoints; drained by the DAP server.
    pub pending_output: Vec<String>,
}

impl Stepper {
    /// Create a new stepper for the given program.
    pub fn new(
        engine: Engine,
        program: Vec<Spanned<Expr>>,
        source: String,
        source_path: String,
    ) -> Self {
        let line_index = LineIndex::new(&source);
        let frames = vec![StepFrame {
            name: "<program>".into(),
            source_path: source_path.clone(),
            body: program,
            pc: 0,
        }];
        Stepper {
            engine,
            frames,
            source_path,
            source,
            line_index,
            breakpoints: HashMap::new(),
            pending_output: Vec::new(),
        }
    }

    // ── Breakpoint management ─────────────────────────────────────────────

    /// Replace all breakpoints.
    pub fn set_breakpoints(&mut self, bps: Vec<Breakpoint>) {
        self.breakpoints.clear();
        for bp in bps {
            self.breakpoints.insert(bp.line, bp);
        }
    }

    /// Returns the breakpoint at the *current* (not-yet-executed) position.
    pub fn breakpoint_at_current(&self) -> Option<&Breakpoint> {
        let frame = self.frames.last()?;
        let span = frame.current_span()?;
        let line = self.line_index.line_of(span.start) + 1; // 1-based
        self.breakpoints.get(&line)
    }

    /// Returns the 1-based line of the current expression, or `None`.
    pub fn current_line(&self) -> Option<usize> {
        let frame = self.frames.last()?;
        let span = frame.current_span()?;
        Some(self.line_index.line_of(span.start) + 1)
    }

    // ── Stepping ─────────────────────────────────────────────────────────

    /// Execute the current expression atomically and advance the program counter.
    pub fn step_over(&mut self) -> StepperState {
        self.drain_exhausted_frames();
        if self.frames.is_empty() {
            return StepperState::Terminated;
        }

        let frame = self.frames.last_mut().unwrap();
        let spanned = frame.body[frame.pc].clone();
        frame.pc += 1;

        match self.engine.eval_one(&spanned) {
            Ok(()) => StepperState::Paused(StopReason::Step),
            Err(e) => StepperState::Errored(e),
        }
    }

    /// Step into the current expression.
    ///
    /// If the expression is `Execute` (`!`) and the top of the value stack is a
    /// `Block`, intercept: pop the block and push a new call frame. Otherwise,
    /// behaves identically to `step_over`.
    pub fn step_in(&mut self) -> StepperState {
        self.drain_exhausted_frames();
        if self.frames.is_empty() {
            return StepperState::Terminated;
        }

        let is_execute_into_block = {
            let frame = self.frames.last().unwrap();
            matches!(&frame.body[frame.pc].0, Expr::Execute)
                && matches!(self.engine.peek_top(), Some(Value::Block(_)))
        };

        if is_execute_into_block {
            let (_, span) = {
                let frame = self.frames.last_mut().unwrap();
                let s = frame.body[frame.pc].clone();
                frame.pc += 1;
                s
            };

            let block = match self.engine.pop_value() {
                Some(Value::Block(b)) => b,
                _ => unreachable!("guarded above"),
            };
            self.frames.push(StepFrame {
                name: format!("block@{}", span.start),
                source_path: self.source_path.clone(),
                body: block.body,
                pc: 0,
            });
            return StepperState::Paused(StopReason::Step);
        }

        // Fall through: step over
        let frame = self.frames.last_mut().unwrap();
        let spanned = frame.body[frame.pc].clone();
        frame.pc += 1;

        match self.engine.eval_one(&spanned) {
            Ok(()) => StepperState::Paused(StopReason::Step),
            Err(e) => StepperState::Errored(e),
        }
    }

    /// Execute to the end of the current call frame, then pause in the parent.
    pub fn step_out(&mut self) -> StepperState {
        if self.frames.is_empty() {
            return StepperState::Terminated;
        }
        let target_depth = self.frames.len().saturating_sub(1);

        loop {
            match self.step_over() {
                StepperState::Terminated => return StepperState::Terminated,
                StepperState::Errored(e) => return StepperState::Errored(e),
                StepperState::Paused(_) => {
                    if self.frames.len() <= target_depth {
                        return if self.frames.is_empty() {
                            StepperState::Terminated
                        } else {
                            StepperState::Paused(StopReason::Step)
                        };
                    }
                }
            }
        }
    }

    /// Run until the next breakpoint, an error, or program termination.
    ///
    /// Starts by executing the current instruction first so we don't re-trigger
    /// the breakpoint we're already paused at.
    pub fn continue_execution(&mut self) -> StepperState {
        loop {
            match self.step_over() {
                StepperState::Terminated => return StepperState::Terminated,
                StepperState::Errored(e) => return StepperState::Errored(e),
                StepperState::Paused(_) => {
                    // Clone breakpoint data before any mutable borrows to satisfy
                    // the borrow checker (eval_condition borrows self mutably).
                    let bp_data: Option<(usize, Option<String>, Option<String>)> = self
                        .breakpoint_at_current()
                        .map(|bp| (bp.line, bp.condition.clone(), bp.log_message.clone()));

                    if let Some((line, condition, log_message)) = bp_data {
                        // Conditional breakpoint: eval in engine state snapshot
                        if let Some(cond) = condition {
                            match self.eval_condition(&cond) {
                                Ok(true) => {}                  // halt
                                Ok(false) | Err(_) => continue, // skip
                            }
                        }
                        // Logpoint: emit message, do not halt
                        if let Some(msg) = log_message {
                            self.pending_output.push(msg);
                            continue;
                        }
                        return StepperState::Paused(StopReason::Breakpoint(line));
                    }
                }
            }
        }
    }

    /// Drain any log messages queued by logpoint breakpoints.
    pub fn drain_pending_output(&mut self) -> Vec<String> {
        std::mem::take(&mut self.pending_output)
    }

    // ── Inspection ────────────────────────────────────────────────────────

    /// Returns the current call stack, innermost frame first (DAP convention).
    pub fn stack_frames(&self) -> Vec<FrameInfo> {
        self.frames
            .iter()
            .enumerate()
            .rev()
            .map(|(id, frame)| {
                let (line, col) = frame
                    .current_span()
                    .map(|span| {
                        let (l, c) = self.line_index.line_col(span.start);
                        (l + 1, c + 1) // 1-based for DAP
                    })
                    .unwrap_or((1, 1));
                FrameInfo {
                    id,
                    name: frame.name.clone(),
                    source_path: frame.source_path.clone(),
                    line,
                    column: col,
                }
            })
            .collect()
    }

    /// Returns all named bindings visible in the current scope chain,
    /// inner scopes shadowing outer ones. Sorted by name.
    pub fn scope_variables(&self) -> Vec<ScopeVariable> {
        let env = self.engine.env_snapshot();
        let mut seen = std::collections::HashSet::new();
        let mut vars: Vec<ScopeVariable> = Vec::new();
        for scope in env.iter().rev() {
            for (name, value) in scope {
                if seen.insert(name.clone()) {
                    vars.push(make_scope_var(name.clone(), value));
                }
            }
        }
        vars.sort_by(|a, b| a.name.cmp(&b.name));
        vars
    }

    /// Returns the current value stack as indexed variables (bottom first, TOS last).
    pub fn stack_variables(&self) -> Vec<ScopeVariable> {
        self.engine
            .stack()
            .iter()
            .enumerate()
            .map(|(i, v)| make_scope_var(format!("[{i}]"), v))
            .collect()
    }

    // ── Internal ─────────────────────────────────────────────────────────

    fn drain_exhausted_frames(&mut self) {
        while let Some(frame) = self.frames.last() {
            if frame.is_exhausted() {
                self.frames.pop();
            } else {
                break;
            }
        }
    }

    /// Evaluate a condition expression in a snapshot of the current engine
    /// state so the condition cannot corrupt the running program.
    fn eval_condition(&mut self, condition: &str) -> Result<bool, EvalError> {
        let tokens = crate::lexer::lex(condition).map_err(|_| EvalError {
            kind: crate::error::EvalErrorKind::IoError("bad condition syntax".into()),
            span: None,
            labels: vec![],
        })?;
        let ast = crate::parser::parse(&tokens, condition.len()).map_err(|_| EvalError {
            kind: crate::error::EvalErrorKind::IoError("bad condition syntax".into()),
            span: None,
            labels: vec![],
        })?;

        // Save and restore the stack so the condition is side-effect-free
        let saved_stack = self.engine.clone_raw_stack();
        let eval_result = self.engine.eval(&ast);
        let truthy = self
            .engine
            .pop_value()
            .map(|v| v.is_truthy())
            .unwrap_or(false);
        self.engine.set_stack(saved_stack);
        eval_result?;
        Ok(truthy)
    }
}

// ── Helpers ───────────────────────────────────────────────────────────────

fn make_scope_var(name: String, value: &Value) -> ScopeVariable {
    ScopeVariable {
        name,
        value: format_value(value),
        type_name: value.type_name().into(),
        children: value_children(value),
    }
}

fn format_value(v: &Value) -> String {
    match v {
        Value::List(items) => format!("[{} items]", items.len()),
        Value::Block(b) => format!("(block, {} exprs)", b.body.len()),
        other => other.to_string(),
    }
}

fn value_children(v: &Value) -> Vec<ScopeVariable> {
    match v {
        Value::List(items) => items
            .iter()
            .enumerate()
            .map(|(i, item)| make_scope_var(format!("[{i}]"), item))
            .collect(),
        _ => vec![],
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::eval::Engine;

    fn parse(src: &str) -> Vec<Spanned<Expr>> {
        let tokens = crate::lexer::lex(src).unwrap();
        crate::parser::parse(&tokens, src.len()).unwrap()
    }

    #[test]
    fn step_over_simple() {
        let src = "1 2 +";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());

        s.step_over(); // push 1
        assert_eq!(s.engine.stack().len(), 1);

        s.step_over(); // push 2
        assert_eq!(s.engine.stack().len(), 2);

        s.step_over(); // +
        assert_eq!(s.engine.stack().len(), 1);
        assert_eq!(s.engine.stack()[0].to_string(), "3");

        let state = s.step_over(); // exhausted
        assert!(matches!(state, StepperState::Terminated));
    }

    #[test]
    fn step_in_block() {
        let src = "(3 4 +) !";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());

        s.step_over(); // push the block (1 frame)
        assert_eq!(s.frames.len(), 1);
        assert_eq!(s.engine.stack().len(), 1); // block on value stack

        // step_in should intercept Execute and push a new frame
        let state = s.step_in();
        assert!(matches!(state, StepperState::Paused(StopReason::Step)));
        assert_eq!(s.frames.len(), 2);
        assert_eq!(s.engine.stack().len(), 0); // block was popped

        s.step_over(); // 3
        s.step_over(); // 4
        s.step_over(); // +
        assert_eq!(s.engine.stack()[0].to_string(), "7");
    }

    #[test]
    fn breakpoint_halts() {
        let src = "1\n2\n3\n+\n+";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        s.set_breakpoints(vec![Breakpoint {
            line: 3,
            condition: None,
            log_message: None,
        }]);

        let state = s.continue_execution();
        assert!(matches!(
            state,
            StepperState::Paused(StopReason::Breakpoint(3))
        ));
        assert_eq!(s.engine.stack().len(), 2); // 1 and 2 pushed before halt
    }

    #[test]
    fn step_out_returns_to_parent() {
        let src = "(1 2 +) !";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());

        s.step_over(); // push block
        s.step_in(); // enter block frame (2 frames total)
        assert_eq!(s.frames.len(), 2);

        let state = s.step_out(); // run to end of block
                                  // Either terminated (if parent was also exhausted) or back at 1 frame
        match state {
            StepperState::Terminated | StepperState::Paused(_) => {}
            StepperState::Errored(e) => panic!("unexpected error: {e:?}"),
        }
        assert_eq!(s.engine.stack()[0].to_string(), "3");
    }
}
