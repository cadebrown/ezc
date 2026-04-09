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
/// - Higher frames are block invocations, loop cond/body slices, `each` / map /
///   filter / fold bodies, or scoped `{...}` bodies.
///
/// `step_over` executes the current expression one step at a time. Composite
/// forms (`&`, `each`, `import`, `[…]`, `?`, `??`, `&!`, `&?`, `&/`, `{…}`) and
/// `!` on a block, string, or list use [`StepDriver`] / child frames so inner
/// work is stepped instead of a whole [`Engine::eval`]. A bare [`Expr::Ident`]
/// that resolves to a bound block is stepped the same way.
///
/// Pushing a block literal `( … )` (without `!`) is still one step — its body
/// runs only when the block is executed.
///
/// `step_in` currently matches `step_over` (same interception rules).
///
/// `step_out` runs to the end of the current frame, then pauses in the parent.
use std::ops::Range;

use crate::ast::{Expr, Spanned};
use crate::error::{EvalError, EvalErrorKind};
use crate::eval::{Engine, ImportStep};
use crate::line_index::LineIndex;
use crate::types::Value;

// ── Loop stepping (`&`) ───────────────────────────────────────────────────

/// State for stepping a `&` loop without atomic evaluation of cond/body blocks.
#[derive(Debug, Clone)]
pub struct LoopDriver {
    pub cond: Vec<Spanned<Expr>>,
    pub body: Vec<Spanned<Expr>>,
    pub phase: LoopPhase,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LoopPhase {
    RunningCond,
    RunningBody,
}

// ── Other composite drivers ───────────────────────────────────────────────

#[derive(Debug, Clone)]
pub struct EachDriver {
    pub items: Vec<Value>,
    /// Index of the element currently on the stack / being stepped.
    pub next_idx: usize,
    pub span: Range<usize>,
}

#[derive(Debug, Clone)]
pub struct MapDriver {
    pub items: Vec<Value>,
    pub next_idx: usize,
    pub out: Vec<Value>,
    pub span: Range<usize>,
}

#[derive(Debug, Clone)]
pub struct FilterDriver {
    pub items: Vec<Value>,
    pub next_idx: usize,
    pub out: Vec<Value>,
    pub span: Range<usize>,
}

#[derive(Debug, Clone)]
pub struct FoldDriver {
    pub items: Vec<Value>,
    /// Number of fold iterations completed; next operand is `items[finished_count]` when `< len`.
    pub finished_count: usize,
    pub span: Range<usize>,
}

#[derive(Debug, Clone)]
pub struct ListBuildDriver {
    pub base_stack: usize,
    pub span: Range<usize>,
}

#[derive(Debug, Clone)]
pub struct SplatDriver {
    pub items: Vec<Value>,
    pub idx: usize,
    pub span: Range<usize>,
}

/// State for stepping composite expressions without atomic `eval` of whole bodies.
#[derive(Debug, Clone)]
pub enum StepDriver {
    Loop(LoopDriver),
    Each(EachDriver),
    Map(MapDriver),
    Filter(FilterDriver),
    Fold(FoldDriver),
    List(ListBuildDriver),
    Splat(SplatDriver),
}

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
    /// When set, this frame is stepping a composite (`&`, `each`, etc.).
    pub driver: Option<StepDriver>,
    /// [`Engine::pop_scope`] calls when this frame is removed after exhaustion.
    pub scopes_to_pop: u8,
    /// Line mapping for `body` spans (may differ per frame for `import` / string `!`).
    pub line_index: LineIndex,
}

impl StepFrame {
    /// Returns the span of the expression *about to be* executed, or `None`
    /// if the frame has nothing left to execute.
    pub fn current_span(&self) -> Option<Range<usize>> {
        if self.pc < self.body.len() {
            return self.body.get(self.pc).map(|(_, span)| span.clone());
        }
        match &self.driver {
            Some(StepDriver::Splat(d)) => Some(d.span.clone()),
            Some(StepDriver::List(d)) => Some(d.span.clone()),
            _ => None,
        }
    }

    /// `true` if this frame can be removed by `drain_exhausted_frames`.
    ///
    /// Frames at `pc == body.len()` with an active driver need a driver
    /// transition first and are not exhausted.
    pub fn is_exhausted(&self) -> bool {
        if self.pc < self.body.len() {
            return false;
        }
        self.driver.is_none()
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
    /// 1-based column (character offset on the line). When `None`, any column on `line` matches.
    pub column: Option<u32>,
    /// When set, only applies while the current stack frame's [`StepFrame::source_path`] matches
    /// (after normalizing path separators). `None` matches every frame (typical for tests).
    pub source_path: Option<String>,
    /// Optional EZC condition expression. Must leave a truthy value on the
    /// stack to trigger a halt. Evaluated in a snapshot of engine state so
    /// the condition cannot corrupt the running program.
    pub condition: Option<String>,
    /// Logpoint message (no halt — emits to the debug console instead).
    pub log_message: Option<String>,
    /// Hit condition expression (e.g. "5" means break on 5th hit, ">3" means after 3rd).
    /// Supported forms: "N" (== N), ">N", ">=N", "<N", "<=N", "%N" (every Nth).
    pub hit_condition: Option<String>,
    /// Number of times this breakpoint has been hit (for hit count evaluation).
    pub hit_count: usize,
}

impl Breakpoint {
    /// Breakpoint on a 1-based line, any column, any source file.
    pub fn line_only(line: usize) -> Self {
        Self {
            line,
            column: None,
            source_path: None,
            condition: None,
            log_message: None,
            hit_condition: None,
            hit_count: 0,
        }
    }
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
    /// Active breakpoints (order preserved; first match wins in [`Stepper::breakpoint_at_current`]).
    pub breakpoints: Vec<Breakpoint>,
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
            driver: None,
            scopes_to_pop: 0,
            line_index: line_index.clone(),
        }];
        Stepper {
            engine,
            frames,
            source_path,
            source,
            line_index,
            breakpoints: Vec::new(),
            pending_output: Vec::new(),
        }
    }

    // ── Breakpoint management ─────────────────────────────────────────────

    /// Replace all breakpoints.
    pub fn set_breakpoints(&mut self, bps: Vec<Breakpoint>) {
        self.breakpoints = bps;
    }

    /// Returns the breakpoint at the *current* (not-yet-executed) position.
    pub fn breakpoint_at_current(&self) -> Option<&Breakpoint> {
        let frame = self.frames.last()?;
        let span = frame.current_span()?;
        let (l0, c0) = frame.line_index.line_col(span.start);
        let line = l0 + 1;
        let col = (c0 + 1) as u32;
        self.breakpoints.iter().find(|bp| {
            breakpoint_source_matches(bp.source_path.as_deref(), &frame.source_path)
                && bp.line == line
                && match bp.column {
                    None => true,
                    Some(bc) => bc == col,
                }
        })
    }

    /// Returns the 1-based line of the current expression, or `None`.
    pub fn current_line(&self) -> Option<usize> {
        let frame = self.frames.last()?;
        let span = frame.current_span()?;
        Some(frame.line_index.line_of(span.start) + 1)
    }

    /// Increment the current frame's `pc` and clone its `source_path` and `line_index` for a
    /// child frame that uses the same on-disk source mapping.
    fn advance_pc_clone_source_context(&mut self) -> Option<(String, LineIndex)> {
        let frame = self.frames.last_mut()?;
        let source_path = frame.source_path.clone();
        let line_index = frame.line_index.clone();
        frame.pc += 1;
        Some((source_path, line_index))
    }

    // ── Stepping ─────────────────────────────────────────────────────────

    /// After a composite body's slice finishes, advance drivers (loop phases,
    /// next `each` item, etc.).
    fn advance_composite_drivers(&mut self) -> Result<(), EvalError> {
        let Some(frame) = self.frames.last_mut() else {
            return Ok(());
        };
        if frame.driver.is_none() || frame.pc < frame.body.len() {
            return Ok(());
        }

        match frame.driver.as_mut() {
            None => return Ok(()),
            Some(driver) => match driver {
                StepDriver::Loop(driver) => {
                    let span_hint = driver.cond.first().map(|(_, s)| s.clone()).unwrap_or(0..0);
                    match driver.phase {
                        LoopPhase::RunningCond => {
                            let flag = self.engine.pop_value().ok_or_else(|| EvalError {
                                kind: EvalErrorKind::StackUnderflow {
                                    op: "& (condition result)".into(),
                                    expected: 1,
                                    found: 0,
                                },
                                span: Some(span_hint),
                                labels: vec![],
                            })?;
                            if !flag.is_truthy() {
                                frame.driver = None;
                                self.frames.pop();
                            } else {
                                driver.phase = LoopPhase::RunningBody;
                                frame.body = driver.body.clone();
                                frame.pc = 0;
                            }
                        }
                        LoopPhase::RunningBody => {
                            driver.phase = LoopPhase::RunningCond;
                            frame.body = driver.cond.clone();
                            frame.pc = 0;
                        }
                    }
                }
                StepDriver::Each(d) => {
                    let span = d.span.clone();
                    d.next_idx += 1;
                    if d.next_idx < d.items.len() {
                        self.engine.push(d.items[d.next_idx].clone(), span.clone());
                        frame.pc = 0;
                    } else {
                        frame.driver = None;
                    }
                }
                StepDriver::Map(d) => {
                    let span = d.span.clone();
                    let mapped = self.engine.pop_value().ok_or_else(|| EvalError {
                        kind: EvalErrorKind::StackUnderflow {
                            op: "&! (map body produced no value)".into(),
                            expected: 1,
                            found: 0,
                        },
                        span: Some(span.clone()),
                        labels: vec![],
                    })?;
                    d.out.push(mapped);
                    d.next_idx += 1;
                    if d.next_idx < d.items.len() {
                        self.engine.push(d.items[d.next_idx].clone(), span.clone());
                        frame.pc = 0;
                    } else {
                        self.engine
                            .push(Value::List(std::mem::take(&mut d.out)), span);
                        frame.driver = None;
                    }
                }
                StepDriver::Filter(d) => {
                    let span = d.span.clone();
                    let keep = self
                        .engine
                        .pop_value()
                        .map(|v| v.is_truthy())
                        .unwrap_or(false);
                    if keep {
                        let item = d.items[d.next_idx].clone();
                        d.out.push(item);
                    }
                    d.next_idx += 1;
                    if d.next_idx < d.items.len() {
                        self.engine.push(d.items[d.next_idx].clone(), span.clone());
                        frame.pc = 0;
                    } else {
                        self.engine
                            .push(Value::List(std::mem::take(&mut d.out)), span);
                        frame.driver = None;
                    }
                }
                StepDriver::Fold(d) => {
                    let span = d.span.clone();
                    d.finished_count += 1;
                    if d.finished_count < d.items.len() {
                        self.engine
                            .push(d.items[d.finished_count].clone(), span.clone());
                        frame.pc = 0;
                    } else {
                        frame.driver = None;
                    }
                }
                StepDriver::List(d) => {
                    let span = d.span.clone();
                    let base = d.base_stack;
                    self.engine.finalize_list_literal(base, span)?;
                    frame.driver = None;
                }
                StepDriver::Splat(d) => {
                    let span = d.span.clone();
                    if d.idx < d.items.len() {
                        self.engine.push(d.items[d.idx].clone(), span.clone());
                        d.idx += 1;
                        if d.idx >= d.items.len() {
                            frame.driver = None;
                        }
                    } else {
                        frame.driver = None;
                    }
                }
            },
        }
        Ok(())
    }

    fn try_begin_loop_stepping(&mut self) -> Option<StepperState> {
        let spanned = {
            let frame = self.frames.last()?;
            if frame.pc >= frame.body.len() {
                return None;
            }
            let sp = frame.body[frame.pc].clone();
            if !matches!(sp.0, Expr::Loop) {
                return None;
            }
            sp
        };

        let span = spanned.1.clone();
        match self.engine.pop_loop_blocks(span.clone()) {
            Ok((cond_b, body_b)) => {
                let (source_path, line_index) = self.advance_pc_clone_source_context()?;
                self.frames.push(StepFrame {
                    name: format!("loop@{}", span.start),
                    source_path,
                    body: cond_b.body.clone(),
                    pc: 0,
                    driver: Some(StepDriver::Loop(LoopDriver {
                        cond: cond_b.body,
                        body: body_b.body,
                        phase: LoopPhase::RunningCond,
                    })),
                    scopes_to_pop: 0,
                    line_index,
                });
                Some(StepperState::Paused(StopReason::Step))
            }
            Err(e) => Some(StepperState::Errored(e)),
        }
    }

    fn try_begin_scope(&mut self) -> Option<StepperState> {
        let inner = {
            let frame = self.frames.last()?;
            if frame.pc >= frame.body.len() {
                return None;
            }
            match &frame.body[frame.pc].0 {
                Expr::Scope(b) => b.clone(),
                _ => return None,
            }
        };
        let span = self.frames.last().map(|f| f.body[f.pc].1.clone())?;
        let (source_path, line_index) = self.advance_pc_clone_source_context()?;
        self.engine.push_scope();
        self.frames.push(StepFrame {
            name: format!("scope@{}", span.start),
            source_path,
            body: inner,
            pc: 0,
            driver: None,
            scopes_to_pop: 1,
            line_index,
        });
        Some(StepperState::Paused(StopReason::Step))
    }

    fn try_begin_each(&mut self) -> Option<StepperState> {
        if !self.matching_builtin("each") {
            return None;
        }
        let span = self.frames.last().map(|f| f.body[f.pc].1.clone())?;
        match self.engine.pop_each_operands(span.clone()) {
            Ok((items, block)) => {
                let (source_path, line_index) = self.advance_pc_clone_source_context()?;
                if items.is_empty() {
                    return Some(StepperState::Paused(StopReason::Step));
                }
                self.engine.push(items[0].clone(), span.clone());
                self.frames.push(StepFrame {
                    name: format!("each@{}", span.start),
                    source_path,
                    body: block.body.clone(),
                    pc: 0,
                    driver: Some(StepDriver::Each(EachDriver {
                        items,
                        next_idx: 0,
                        span: span.clone(),
                    })),
                    scopes_to_pop: 0,
                    line_index,
                });
                Some(StepperState::Paused(StopReason::Step))
            }
            Err(e) => Some(StepperState::Errored(e)),
        }
    }

    fn try_begin_map(&mut self) -> Option<StepperState> {
        if !matches!(self.current_expr_head(), Some(ExprHead::Map)) {
            return None;
        }
        let span = self.frames.last().map(|f| f.body[f.pc].1.clone())?;
        match self.engine.pop_map_operands(span.clone()) {
            Ok((items, block)) => {
                let (source_path, line_index) = self.advance_pc_clone_source_context()?;
                if items.is_empty() {
                    self.engine.push(Value::List(vec![]), span);
                    return Some(StepperState::Paused(StopReason::Step));
                }
                self.engine.push(items[0].clone(), span.clone());
                self.frames.push(StepFrame {
                    name: format!("map@{}", span.start),
                    source_path,
                    body: block.body.clone(),
                    pc: 0,
                    driver: Some(StepDriver::Map(MapDriver {
                        items,
                        next_idx: 0,
                        out: Vec::new(),
                        span: span.clone(),
                    })),
                    scopes_to_pop: 0,
                    line_index,
                });
                Some(StepperState::Paused(StopReason::Step))
            }
            Err(e) => Some(StepperState::Errored(e)),
        }
    }

    fn try_begin_filter(&mut self) -> Option<StepperState> {
        if !matches!(self.current_expr_head(), Some(ExprHead::Filter)) {
            return None;
        }
        let span = self.frames.last().map(|f| f.body[f.pc].1.clone())?;
        match self.engine.pop_filter_operands(span.clone()) {
            Ok((items, block)) => {
                let (source_path, line_index) = self.advance_pc_clone_source_context()?;
                if items.is_empty() {
                    self.engine.push(Value::List(vec![]), span);
                    return Some(StepperState::Paused(StopReason::Step));
                }
                self.engine.push(items[0].clone(), span.clone());
                self.frames.push(StepFrame {
                    name: format!("filter@{}", span.start),
                    source_path,
                    body: block.body.clone(),
                    pc: 0,
                    driver: Some(StepDriver::Filter(FilterDriver {
                        items,
                        next_idx: 0,
                        out: Vec::new(),
                        span: span.clone(),
                    })),
                    scopes_to_pop: 0,
                    line_index,
                });
                Some(StepperState::Paused(StopReason::Step))
            }
            Err(e) => Some(StepperState::Errored(e)),
        }
    }

    fn try_begin_fold(&mut self) -> Option<StepperState> {
        if !matches!(self.current_expr_head(), Some(ExprHead::Fold)) {
            return None;
        }
        let span = self.frames.last().map(|f| f.body[f.pc].1.clone())?;
        match self.engine.pop_fold_operands(span.clone()) {
            Ok((items, init, block)) => {
                let (source_path, line_index) = self.advance_pc_clone_source_context()?;
                self.engine.push(init.value, init.span.clone());
                if items.is_empty() {
                    return Some(StepperState::Paused(StopReason::Step));
                }
                self.engine.push(items[0].clone(), span.clone());
                self.frames.push(StepFrame {
                    name: format!("fold@{}", span.start),
                    source_path,
                    body: block.body.clone(),
                    pc: 0,
                    driver: Some(StepDriver::Fold(FoldDriver {
                        items,
                        finished_count: 0,
                        span: span.clone(),
                    })),
                    scopes_to_pop: 0,
                    line_index,
                });
                Some(StepperState::Paused(StopReason::Step))
            }
            Err(e) => Some(StepperState::Errored(e)),
        }
    }

    fn try_enter_execute_block(&mut self) -> Option<StepperState> {
        let is_execute_into_block = {
            let frame = self.frames.last()?;
            frame.pc < frame.body.len()
                && matches!(&frame.body[frame.pc].0, Expr::Execute)
                && matches!(self.engine.peek_top(), Some(Value::Block(_)))
        };
        if !is_execute_into_block {
            return None;
        }
        let span = self.frames.last().map(|f| f.body[f.pc].1.clone())?;
        let (source_path, line_index) = self.advance_pc_clone_source_context()?;
        let block = match self.engine.pop_value() {
            Some(Value::Block(b)) => b,
            _ => return None,
        };
        self.frames.push(StepFrame {
            name: format!("block@{}", span.start),
            source_path,
            body: block.body,
            pc: 0,
            driver: None,
            scopes_to_pop: 0,
            line_index,
        });
        Some(StepperState::Paused(StopReason::Step))
    }

    fn try_begin_list_literal(&mut self) -> Option<StepperState> {
        let inner = {
            let frame = self.frames.last()?;
            if frame.pc >= frame.body.len() {
                return None;
            }
            match &frame.body[frame.pc].0 {
                Expr::List(b) => b.clone(),
                _ => return None,
            }
        };
        let span = self.frames.last().map(|f| f.body[f.pc].1.clone())?;
        let (source_path, line_index) = self.advance_pc_clone_source_context()?;
        if inner.is_empty() {
            self.engine.push(Value::List(vec![]), span.clone());
            return Some(StepperState::Paused(StopReason::Step));
        }
        let base_stack = self.engine.stack_len();
        self.frames.push(StepFrame {
            name: format!("list@{}", span.start),
            source_path,
            body: inner,
            pc: 0,
            driver: Some(StepDriver::List(ListBuildDriver {
                base_stack,
                span: span.clone(),
            })),
            scopes_to_pop: 0,
            line_index,
        });
        Some(StepperState::Paused(StopReason::Step))
    }

    fn try_begin_string_execute(&mut self) -> Option<StepperState> {
        let is_str = {
            let frame = self.frames.last()?;
            frame.pc < frame.body.len()
                && matches!(&frame.body[frame.pc].0, Expr::Execute)
                && matches!(self.engine.peek_top(), Some(Value::Str(_)))
        };
        if !is_str {
            return None;
        }
        let span = self.frames.last().map(|f| f.body[f.pc].1.clone())?;
        {
            let frame = self.frames.last_mut()?;
            frame.pc += 1;
        }
        let snippet_src = match self.engine.pop_value() {
            Some(Value::Str(s)) => s.0.to_string(),
            _ => return None,
        };
        match Engine::parse_snippet_ast(&snippet_src, span.clone(), "!") {
            Ok(ast) => {
                let snippet_li = LineIndex::new(&snippet_src);
                self.frames.push(StepFrame {
                    name: format!("string!@{}", span.start),
                    source_path: format!("<string ! @{}>", span.start),
                    body: ast,
                    pc: 0,
                    driver: None,
                    scopes_to_pop: 0,
                    line_index: snippet_li,
                });
                Some(StepperState::Paused(StopReason::Step))
            }
            Err(e) => Some(StepperState::Errored(e)),
        }
    }

    fn try_begin_splat(&mut self) -> Option<StepperState> {
        let is_list = {
            let frame = self.frames.last()?;
            frame.pc < frame.body.len()
                && matches!(&frame.body[frame.pc].0, Expr::Execute)
                && matches!(self.engine.peek_top(), Some(Value::List(_)))
        };
        if !is_list {
            return None;
        }
        let span = self.frames.last().map(|f| f.body[f.pc].1.clone())?;
        let (source_path, line_index) = self.advance_pc_clone_source_context()?;
        let items = match self.engine.pop_value() {
            Some(Value::List(items)) => items,
            _ => return None,
        };
        if items.is_empty() {
            return Some(StepperState::Paused(StopReason::Step));
        }
        self.frames.push(StepFrame {
            name: format!("splat@{}", span.start),
            source_path,
            body: Vec::new(),
            pc: 0,
            driver: Some(StepDriver::Splat(SplatDriver {
                items,
                idx: 0,
                span,
            })),
            scopes_to_pop: 0,
            line_index,
        });
        Some(StepperState::Paused(StopReason::Step))
    }

    fn try_begin_import(&mut self) -> Option<StepperState> {
        if !self.matching_builtin("import") {
            return None;
        }
        let span = self.frames.last().map(|f| f.body[f.pc].1.clone())?;
        match self.engine.prepare_import_step(span.clone()) {
            Ok(ImportStep::Skipped) => {
                let frame = self.frames.last_mut()?;
                frame.pc += 1;
                Some(StepperState::Paused(StopReason::Step))
            }
            Ok(ImportStep::Run {
                module_name,
                module_src,
                ast,
            }) => {
                let frame = self.frames.last_mut()?;
                frame.pc += 1;
                let li = LineIndex::new(&module_src);
                self.frames.push(StepFrame {
                    name: format!("import:{module_name}"),
                    source_path: module_name.clone(),
                    body: ast,
                    pc: 0,
                    driver: None,
                    scopes_to_pop: 0,
                    line_index: li,
                });
                Some(StepperState::Paused(StopReason::Step))
            }
            Err(e) => Some(StepperState::Errored(e)),
        }
    }

    fn try_begin_cond(&mut self) -> Option<StepperState> {
        let frame = self.frames.last()?;
        if frame.pc >= frame.body.len() {
            return None;
        }
        if !matches!(frame.body[frame.pc].0, Expr::Cond) {
            return None;
        }
        let span = frame.body[frame.pc].1.clone();
        let (source_path, line_index) = self.advance_pc_clone_source_context()?;
        match self.engine.pop_cond_operands(&span) {
            Ok((cond, block)) => {
                if !cond.value.is_truthy() {
                    return Some(StepperState::Paused(StopReason::Step));
                }
                match block.value {
                    Value::Block(b) => {
                        self.frames.push(StepFrame {
                            name: format!("?@{}", span.start),
                            source_path,
                            body: b.body,
                            pc: 0,
                            driver: None,
                            scopes_to_pop: 0,
                            line_index,
                        });
                    }
                    other => {
                        self.engine.push(other, block.span);
                    }
                }
                Some(StepperState::Paused(StopReason::Step))
            }
            Err(e) => Some(StepperState::Errored(e)),
        }
    }

    fn try_begin_ternary(&mut self) -> Option<StepperState> {
        let frame = self.frames.last()?;
        if frame.pc >= frame.body.len() {
            return None;
        }
        if !matches!(frame.body[frame.pc].0, Expr::Ternary) {
            return None;
        }
        let span = frame.body[frame.pc].1.clone();
        let (source_path, line_index) = self.advance_pc_clone_source_context()?;
        match self.engine.pop_ternary_operands(&span) {
            Ok((cond, then_b, else_b)) => {
                let chosen = if cond.value.is_truthy() {
                    then_b
                } else {
                    else_b
                };
                match chosen.value {
                    Value::Block(b) => {
                        self.frames.push(StepFrame {
                            name: format!("??@{}", span.start),
                            source_path,
                            body: b.body,
                            pc: 0,
                            driver: None,
                            scopes_to_pop: 0,
                            line_index,
                        });
                    }
                    other => {
                        self.engine.push(other, chosen.span);
                    }
                }
                Some(StepperState::Paused(StopReason::Step))
            }
            Err(e) => Some(StepperState::Errored(e)),
        }
    }

    fn try_begin_bare_block(&mut self) -> Option<StepperState> {
        let name = {
            let frame = self.frames.last()?;
            if frame.pc >= frame.body.len() {
                return None;
            }
            match &frame.body[frame.pc].0 {
                Expr::Ident(n) => n.clone(),
                _ => return None,
            }
        };
        let block = self.engine.lookup_autoexec_block(&name)?;
        let span = self.frames.last().map(|f| f.body[f.pc].1.clone())?;
        let (source_path, line_index) = self.advance_pc_clone_source_context()?;
        self.frames.push(StepFrame {
            name: format!("{name}@{}", span.start),
            source_path,
            body: block.body,
            pc: 0,
            driver: None,
            scopes_to_pop: 0,
            line_index,
        });
        Some(StepperState::Paused(StopReason::Step))
    }

    fn matching_builtin(&self, name: &str) -> bool {
        let Some(frame) = self.frames.last() else {
            return false;
        };
        if frame.pc >= frame.body.len() {
            return false;
        }
        matches!(&frame.body[frame.pc].0, Expr::Ident(s) if s == name)
    }

    fn current_expr_head(&self) -> Option<ExprHead> {
        let frame = self.frames.last()?;
        if frame.pc >= frame.body.len() {
            return None;
        }
        match &frame.body[frame.pc].0 {
            Expr::Map => Some(ExprHead::Map),
            Expr::Ident(s) if s == "map" => Some(ExprHead::Map),
            Expr::Filter => Some(ExprHead::Filter),
            Expr::Ident(s) if s == "fil" => Some(ExprHead::Filter),
            Expr::Fold => Some(ExprHead::Fold),
            Expr::Ident(s) if s == "red" => Some(ExprHead::Fold),
            _ => None,
        }
    }

    /// Execute the current expression atomically and advance the program counter.
    pub fn step_over(&mut self) -> StepperState {
        if let Err(e) = self.advance_composite_drivers() {
            return StepperState::Errored(e);
        }
        self.drain_exhausted_frames();
        if self.frames.is_empty() {
            return StepperState::Terminated;
        }

        if let Some(s) = self.try_begin_loop_stepping() {
            return s;
        }
        if let Some(s) = self.try_begin_scope() {
            return s;
        }
        if let Some(s) = self.try_begin_each() {
            return s;
        }
        if let Some(s) = self.try_begin_map() {
            return s;
        }
        if let Some(s) = self.try_begin_filter() {
            return s;
        }
        if let Some(s) = self.try_begin_fold() {
            return s;
        }
        if let Some(s) = self.try_begin_list_literal() {
            return s;
        }
        if let Some(s) = self.try_enter_execute_block() {
            return s;
        }
        if let Some(s) = self.try_begin_string_execute() {
            return s;
        }
        if let Some(s) = self.try_begin_splat() {
            return s;
        }
        if let Some(s) = self.try_begin_import() {
            return s;
        }
        if let Some(s) = self.try_begin_cond() {
            return s;
        }
        if let Some(s) = self.try_begin_ternary() {
            return s;
        }
        if let Some(s) = self.try_begin_bare_block() {
            return s;
        }

        let frame = self.frames.last_mut().unwrap();
        if frame.pc >= frame.body.len() {
            return StepperState::Paused(StopReason::Step);
        }

        let spanned = frame.body[frame.pc].clone();
        frame.pc += 1;

        match self.engine.eval_one(&spanned) {
            Ok(()) => StepperState::Paused(StopReason::Step),
            Err(e) => StepperState::Errored(e),
        }
    }

    /// Step into the current expression.
    ///
    /// Same interception rules as [`Stepper::step_over`] (including `!` into blocks).
    pub fn step_in(&mut self) -> StepperState {
        self.step_over()
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
                    // Find the matching breakpoint index so we can mutate hit_count.
                    let bp_idx = self.matching_breakpoint_index();
                    let Some(idx) = bp_idx else { continue };

                    // Increment hit count.
                    self.breakpoints[idx].hit_count += 1;
                    let bp = &self.breakpoints[idx];
                    let line = bp.line;
                    let condition = bp.condition.clone();
                    let log_message = bp.log_message.clone();
                    let hit_condition = bp.hit_condition.clone();
                    let hit_count = bp.hit_count;

                    // Evaluate condition expression (if any).
                    if let Some(cond) = condition {
                        match self.eval_condition(&cond) {
                            Ok(true) => {}
                            Ok(false) => continue,
                            Err(e) => {
                                self.pending_output.push(format!(
                                    "Breakpoint condition error: {}", e.kind
                                ));
                                continue;
                            }
                        }
                    }

                    // Evaluate hit condition (if any).
                    if let Some(ref hc) = hit_condition {
                        match Self::eval_hit_condition(hc, hit_count) {
                            Ok(true) => {}
                            Ok(false) => continue,
                            Err(msg) => {
                                self.pending_output.push(format!(
                                    "Hit condition error: {msg}"
                                ));
                                continue;
                            }
                        }
                    }

                    // Logpoints emit output but don't halt.
                    if let Some(msg) = log_message {
                        self.pending_output.push(msg);
                        continue;
                    }

                    return StepperState::Paused(StopReason::Breakpoint(line));
                }
            }
        }
    }

    /// Find the index of the breakpoint matching the current position, or `None`.
    fn matching_breakpoint_index(&self) -> Option<usize> {
        let frame = self.frames.last()?;
        let span = frame.current_span()?;
        let (l0, c0) = frame.line_index.line_col(span.start);
        let line = l0 + 1;
        let col = (c0 + 1) as u32;
        self.breakpoints.iter().position(|bp| {
            breakpoint_source_matches(bp.source_path.as_deref(), &frame.source_path)
                && bp.line == line
                && match bp.column {
                    None => true,
                    Some(bc) => bc == col,
                }
        })
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
                        let (l, c) = frame.line_index.line_col(span.start);
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
                let popped = self.frames.pop().expect("last");
                for _ in 0..popped.scopes_to_pop {
                    self.engine.pop_scope();
                }
            } else {
                break;
            }
        }
    }

    /// Evaluate a hit condition expression against the current hit count.
    /// Returns `true` if the breakpoint should fire.
    ///
    /// Supported forms:
    /// - `"5"` or `"== 5"` — fire on exactly the 5th hit
    /// - `"> 3"` — fire when hit count exceeds 3
    /// - `">= 3"` — fire when hit count is 3 or more
    /// - `"< 3"` — fire when hit count is less than 3
    /// - `"<= 3"` — fire when hit count is 3 or fewer
    /// - `"% 3"` — fire every 3rd hit (modulo)
    fn eval_hit_condition(expr: &str, hit_count: usize) -> Result<bool, String> {
        let expr = expr.trim();
        if expr.is_empty() {
            return Err("empty hit condition".into());
        }

        // Try to parse as plain number first (== N)
        if let Ok(n) = expr.parse::<usize>() {
            return Ok(hit_count == n);
        }

        // Parse operator + number
        let (op, rest) = if let Some(r) = expr.strip_prefix(">=") {
            (">=", r)
        } else if let Some(r) = expr.strip_prefix("<=") {
            ("<=", r)
        } else if let Some(r) = expr.strip_prefix("==") {
            ("==", r)
        } else if let Some(r) = expr.strip_prefix('>') {
            (">", r)
        } else if let Some(r) = expr.strip_prefix('<') {
            ("<", r)
        } else if let Some(r) = expr.strip_prefix('%') {
            ("%", r)
        } else {
            return Err(format!("invalid hit condition: `{expr}`"));
        };

        let n: usize = rest.trim().parse().map_err(|_| {
            format!("invalid number in hit condition: `{expr}`")
        })?;

        Ok(match op {
            "==" => hit_count == n,
            ">" => hit_count > n,
            ">=" => hit_count >= n,
            "<" => hit_count < n,
            "<=" => hit_count <= n,
            "%" => n > 0 && hit_count % n == 0,
            _ => false,
        })
    }

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

#[derive(Clone, Copy)]
enum ExprHead {
    Map,
    Filter,
    Fold,
}

// ── Helpers ───────────────────────────────────────────────────────────────

fn breakpoint_source_matches(bp: Option<&str>, frame_path: &str) -> bool {
    match bp {
        None => true,
        Some(p) if p.trim().is_empty() => true,
        Some(p) => crate::debug_source_path::debug_source_paths_equivalent(p, frame_path),
    }
}

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
        Value::Block(b) => b
            .body
            .iter()
            .enumerate()
            .map(|(i, (expr, _))| ScopeVariable {
                name: format!("[{i}]"),
                value: expr_summary(expr),
                type_name: "expr".into(),
                children: vec![],
            })
            .collect(),
        _ => vec![],
    }
}

/// Short human-readable summary of an AST expression (for debugger display).
fn expr_summary(expr: &Expr) -> String {
    match expr {
        Expr::Literal(n) => n.to_string(),
        Expr::StrLiteral(s) => format!("\"{s}\""),
        Expr::Ident(name) => name.clone(),
        Expr::Op(op) => format!("{op}"),
        Expr::Execute => "!".into(),
        Expr::Cond => "?".into(),
        Expr::Ternary => "??".into(),
        Expr::Compose => "|".into(),
        Expr::Loop => "&".into(),
        Expr::Map => "&!".into(),
        Expr::Filter => "&?".into(),
        Expr::Fold => "&/".into(),
        Expr::Equal => "==".into(),
        Expr::NotEqual => "!=".into(),
        Expr::Lt => "<".into(),
        Expr::Gt => ">".into(),
        Expr::LtEq => "<=".into(),
        Expr::GtEq => ">=".into(),
        Expr::Swap => "~".into(),
        Expr::Dup => ",".into(),
        Expr::Drop => ";".into(),
        Expr::Write => ":".into(),
        Expr::Read => ".".into(),
        Expr::Over => "_".into(),
        Expr::Bind(name) => format!("@{name}"),
        Expr::Recall(name) => format!("${name}"),
        Expr::Block(body) => format!("({} exprs)", body.len()),
        Expr::List(body) => format!("[{} exprs]", body.len()),
        Expr::Scope(body) => format!("{{{} exprs}}", body.len()),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::eval::Engine;

    fn bp(line: usize) -> Breakpoint {
        Breakpoint::line_only(line)
    }

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
    fn step_over_enters_execute_block() {
        let src = "(3 4 +) !";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());

        s.step_over(); // push block
        assert_eq!(s.frames.len(), 1);
        assert_eq!(s.engine.stack().len(), 1);

        let state = s.step_over(); // enter block (same as former step_in-only behavior)
        assert!(matches!(state, StepperState::Paused(StopReason::Step)));
        assert_eq!(s.frames.len(), 2);
        assert_eq!(s.engine.stack().len(), 0);

        s.step_over(); // 3
        s.step_over(); // 4
        s.step_over(); // +
        assert_eq!(s.engine.stack()[0].to_string(), "7");
    }

    #[test]
    fn step_in_block() {
        let src = "(3 4 +) !";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());

        s.step_over();
        let state = s.step_in();
        assert!(matches!(state, StepperState::Paused(StopReason::Step)));
        assert_eq!(s.frames.len(), 2);

        s.step_over();
        s.step_over();
        s.step_over();
        assert_eq!(s.engine.stack()[0].to_string(), "7");
    }

    #[test]
    fn breakpoint_halts() {
        let src = "1\n2\n3\n+\n+";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        s.set_breakpoints(vec![bp(3)]);

        let state = s.continue_execution();
        assert!(matches!(
            state,
            StepperState::Paused(StopReason::Breakpoint(3))
        ));
        assert_eq!(s.engine.stack().len(), 2);
    }

    /// Breakpoints inside `&` loop bodies must fire (not skipped via `Engine::eval` on whole blocks).
    #[test]
    fn breakpoint_inside_loop_body() {
        let src = "0 @n\n( $n 3 < )\n(\n$n 1 + @n\n)\n&\n";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        s.set_breakpoints(vec![bp(4)]);

        let state = s.continue_execution();
        assert!(
            matches!(state, StepperState::Paused(StopReason::Breakpoint(4))),
            "expected breakpoint on loop body line, got {state:?}"
        );
    }

    #[test]
    fn breakpoint_inside_each_body() {
        let src = "[1]\n(\n:\n)\neach\n";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        s.set_breakpoints(vec![bp(3)]);

        let state = s.continue_execution();
        assert!(
            matches!(state, StepperState::Paused(StopReason::Breakpoint(3))),
            "expected breakpoint in each body, got {state:?}"
        );
    }

    #[test]
    fn breakpoint_inside_map_body() {
        let src = "[1]\n(\n1 +\n)\n&!\n";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        s.set_breakpoints(vec![bp(3)]);

        let state = s.continue_execution();
        assert!(
            matches!(state, StepperState::Paused(StopReason::Breakpoint(3))),
            "expected breakpoint in map body, got {state:?}"
        );
    }

    #[test]
    fn breakpoint_inside_scope_body() {
        let src = "{\n:\n}\n";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        s.set_breakpoints(vec![bp(2)]);

        let state = s.continue_execution();
        assert!(
            matches!(state, StepperState::Paused(StopReason::Breakpoint(2))),
            "expected breakpoint in scope body, got {state:?}"
        );
    }

    #[test]
    fn breakpoint_inside_filter_body() {
        let src = "[1]\n(\n1\n)\n&?\n";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        s.set_breakpoints(vec![bp(3)]);

        let state = s.continue_execution();
        assert!(
            matches!(state, StepperState::Paused(StopReason::Breakpoint(3))),
            "expected breakpoint in filter body, got {state:?}"
        );
    }

    #[test]
    fn breakpoint_inside_fold_body() {
        let src = "[2]\n0\n(\n+\n)\n&/\n";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        s.set_breakpoints(vec![bp(4)]);

        let state = s.continue_execution();
        assert!(
            matches!(state, StepperState::Paused(StopReason::Breakpoint(4))),
            "expected breakpoint in fold body, got {state:?}"
        );
    }

    #[test]
    fn step_out_returns_to_parent() {
        let src = "(1 2 +) !";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());

        s.step_over();
        s.step_over();
        assert_eq!(s.frames.len(), 2);

        let state = s.step_out();
        match state {
            StepperState::Terminated | StepperState::Paused(_) => {}
            StepperState::Errored(e) => panic!("unexpected error: {e:?}"),
        }
        assert_eq!(s.engine.stack()[0].to_string(), "3");
    }

    #[test]
    fn breakpoint_inside_list_literal() {
        let src = "[\n1\n]\n";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        s.set_breakpoints(vec![bp(2)]);

        let state = s.continue_execution();
        assert!(
            matches!(state, StepperState::Paused(StopReason::Breakpoint(2))),
            "expected breakpoint inside list literal, got {state:?}"
        );
        s.step_over();
        let state = s.continue_execution();
        assert!(matches!(state, StepperState::Terminated), "{state:?}");
    }

    #[test]
    fn breakpoint_in_imported_module() {
        let src = "\"std/math.ezc\" import\n";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "main.ezc".into());
        s.set_breakpoints(vec![Breakpoint {
            line: 4,
            column: None,
            source_path: Some("std/math.ezc".into()),
            condition: None,
            log_message: None,
            hit_condition: None,
            hit_count: 0,
        }]);

        let state = s.continue_execution();
        assert!(
            matches!(state, StepperState::Paused(StopReason::Breakpoint(4))),
            "expected breakpoint in imported module, got {state:?}"
        );
    }

    #[test]
    fn breakpoint_bare_block_call() {
        let src = "(\n1\n)\n@x\nx\n";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        s.set_breakpoints(vec![bp(2)]);

        let state = s.continue_execution();
        assert!(
            matches!(state, StepperState::Paused(StopReason::Breakpoint(2))),
            "expected breakpoint in bare block call body, got {state:?}"
        );
    }

    #[test]
    fn breakpoint_string_execute_multiline() {
        let src = "\"1\n2\n+\" !\n";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        s.set_breakpoints(vec![bp(2)]);

        let state = s.continue_execution();
        assert!(
            matches!(state, StepperState::Paused(StopReason::Breakpoint(2))),
            "expected breakpoint in string-executed code line 2, got {state:?}"
        );
    }

    #[test]
    fn breakpoint_cond_truthy_block() {
        let src = "1 (\n:\n) ?\n";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        s.set_breakpoints(vec![bp(2)]);

        let state = s.continue_execution();
        assert!(
            matches!(state, StepperState::Paused(StopReason::Breakpoint(2))),
            "expected breakpoint in `?` block, got {state:?}"
        );
    }

    /// Falsy `?` must not enter the block — breakpoint inside the block is never hit.
    #[test]
    fn breakpoint_cond_falsy_skips_block() {
        let src = "0 (\n:\n) ?\n";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        s.set_breakpoints(vec![bp(2)]);

        let state = s.continue_execution();
        assert!(
            matches!(state, StepperState::Terminated),
            "expected termination without inner breakpoint, got {state:?}"
        );
    }

    #[test]
    fn breakpoint_ternary_then_block() {
        // Stack bottom→top: cond, then, else — `1` truthy → then block `( : )`
        let src = "1\n(\n:\n)\n(\n;\n)\n??\n";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        s.set_breakpoints(vec![bp(3)]);

        let state = s.continue_execution();
        assert!(
            matches!(state, StepperState::Paused(StopReason::Breakpoint(3))),
            "expected breakpoint in `??` then-branch, got {state:?}"
        );
    }

    #[test]
    fn breakpoint_ternary_else_block() {
        let src = "0\n(\n;\n)\n(\n:\n)\n??\n";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        // Line of `:` in the else block (falsy cond skips then-branch).
        s.set_breakpoints(vec![bp(6)]);

        let state = s.continue_execution();
        assert!(
            matches!(state, StepperState::Paused(StopReason::Breakpoint(6))),
            "expected breakpoint in `??` else-branch, got {state:?}"
        );
    }

    #[test]
    fn string_execute_parse_error_surfaces() {
        let src = "\"(\" !";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        s.step_over();
        let state = s.step_over();
        assert!(
            matches!(state, StepperState::Errored(_)),
            "expected parse error from string `!`, got {state:?}"
        );
    }

    #[test]
    fn splat_execute_steps_each_element() {
        let src = "[1 2] !";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());

        s.step_over(); // list@ frame
        s.step_over(); // push 1 inside list
        s.step_over(); // push 2 inside list
                       // Finalize list, start `!` splat (list is popped off the stack)
        s.step_over();
        assert_eq!(s.engine.stack().len(), 0);
        assert!(s.frames.last().unwrap().name.starts_with("splat@"));
        s.step_over(); // first splat push
        assert_eq!(s.engine.stack().len(), 1);
        assert_eq!(s.engine.stack()[0].to_string(), "1");
        s.step_over();
        assert_eq!(s.engine.stack().len(), 2);
        s.step_over();
        assert!(matches!(s.step_over(), StepperState::Terminated));
    }

    #[test]
    fn breakpoint_column_filters_same_line() {
        let src = "1 2 +";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        // Only halt when the `2` literal is about to run (column is 1-based).
        let col = {
            let span = s.frames[0].body[1].1.clone();
            let (_, c0) = s.line_index.line_col(span.start);
            c0 + 1
        };
        s.set_breakpoints(vec![Breakpoint {
            line: 1,
            column: Some(col as u32),
            source_path: None,
            condition: None,
            log_message: None,
            hit_condition: None,
            hit_count: 0,
        }]);

        // From entry: first `step_over` in continue runs `1`, then we halt before `2`
        // because the breakpoint column matches the next expression.
        let state = s.continue_execution();
        assert!(
            matches!(state, StepperState::Paused(StopReason::Breakpoint(1))),
            "expected column-filtered breakpoint on second expr, got {state:?}"
        );
    }

    // ── Hit condition tests ─────────────────────────────────────────────

    #[test]
    fn hit_condition_exact() {
        assert_eq!(Stepper::eval_hit_condition("3", 3), Ok(true));
        assert_eq!(Stepper::eval_hit_condition("3", 2), Ok(false));
        assert_eq!(Stepper::eval_hit_condition("3", 4), Ok(false));
    }

    #[test]
    fn hit_condition_comparison() {
        assert_eq!(Stepper::eval_hit_condition("> 3", 4), Ok(true));
        assert_eq!(Stepper::eval_hit_condition("> 3", 3), Ok(false));
        assert_eq!(Stepper::eval_hit_condition(">= 3", 3), Ok(true));
        assert_eq!(Stepper::eval_hit_condition("< 3", 2), Ok(true));
        assert_eq!(Stepper::eval_hit_condition("< 3", 3), Ok(false));
        assert_eq!(Stepper::eval_hit_condition("<= 3", 3), Ok(true));
        assert_eq!(Stepper::eval_hit_condition("== 5", 5), Ok(true));
        assert_eq!(Stepper::eval_hit_condition("== 5", 4), Ok(false));
    }

    #[test]
    fn hit_condition_modulo() {
        assert_eq!(Stepper::eval_hit_condition("% 3", 6), Ok(true));
        assert_eq!(Stepper::eval_hit_condition("% 3", 7), Ok(false));
        assert_eq!(Stepper::eval_hit_condition("% 3", 9), Ok(true));
    }

    #[test]
    fn hit_condition_errors() {
        assert!(Stepper::eval_hit_condition("", 1).is_err());
        assert!(Stepper::eval_hit_condition("abc", 1).is_err());
        assert!(Stepper::eval_hit_condition("> abc", 1).is_err());
    }

    #[test]
    fn hit_condition_breakpoint_fires_on_nth_hit() {
        // Loop body executes 5 times. Breakpoint with hitCondition "3" should fire
        // only on the 3rd iteration.
        let src = "0 @n\n( $n 5 < )\n(\n$n 1 + @n\n)\n&\n";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        s.set_breakpoints(vec![Breakpoint {
            line: 4,
            column: None,
            source_path: None,
            condition: None,
            log_message: None,
            hit_condition: Some("3".into()),
            hit_count: 0,
        }]);

        let state = s.continue_execution();
        assert!(
            matches!(state, StepperState::Paused(StopReason::Breakpoint(4))),
            "expected breakpoint on 3rd hit, got {state:?}"
        );
        // n should be 2 at this point (0, 1, 2 — third iteration about to increment)
        assert_eq!(s.breakpoints[0].hit_count, 3);
    }

    // ── Condition error surfacing ───────────────────────────────────────

    #[test]
    fn condition_error_surfaces_as_output() {
        let src = "1\n2\n3\n+\n+";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        s.set_breakpoints(vec![Breakpoint {
            line: 2,
            column: None,
            source_path: None,
            condition: Some("bad syntax @#$".into()),
            log_message: None,
            hit_condition: None,
            hit_count: 0,
        }]);

        // continue_execution should skip the breakpoint (bad condition) and finish
        let state = s.continue_execution();
        // Program terminates (breakpoint never fires due to condition error)
        assert!(matches!(state, StepperState::Terminated));
        // The error message should be in pending_output
        let output = s.drain_pending_output();
        assert!(
            output.iter().any(|msg| msg.contains("condition error")),
            "expected condition error in output, got: {output:?}"
        );
    }

    #[test]
    fn hit_condition_error_surfaces_as_output() {
        let src = "1\n2\n3";
        let program = parse(src);
        let mut s = Stepper::new(Engine::new(), program, src.into(), "test.ezc".into());
        s.set_breakpoints(vec![Breakpoint {
            line: 2,
            column: None,
            source_path: None,
            condition: None,
            log_message: None,
            hit_condition: Some("not_a_number".into()),
            hit_count: 0,
        }]);

        let state = s.continue_execution();
        assert!(matches!(state, StepperState::Terminated));
        let output = s.drain_pending_output();
        assert!(
            output.iter().any(|msg| msg.contains("Hit condition error")),
            "expected hit condition error in output, got: {output:?}"
        );
    }
}
