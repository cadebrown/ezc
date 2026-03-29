use std::collections::{HashMap, HashSet};

use tracing::{debug, trace};

use crate::ast::{Expr, Spanned};
use crate::error::{ErrorLabel, EvalError, EvalErrorKind};
use crate::intern::Interner;
use crate::number::{self, ArithOp, Number};
use crate::token::Op;
use crate::types::{Block, Value};

/// I/O backend for the engine. Default uses stdin/stdout.
pub trait EzIo {
    fn read_line(&mut self) -> std::io::Result<String>;
    fn read_byte(&mut self) -> std::io::Result<u8>;
    fn write_str(&mut self, s: &str) -> std::io::Result<()>;
    fn write_byte(&mut self, b: u8) -> std::io::Result<()>;
}

/// Default I/O using stdin/stdout.
pub struct StdIo;

impl EzIo for StdIo {
    fn read_line(&mut self) -> std::io::Result<String> {
        use std::io::BufRead;
        let mut line = String::new();
        std::io::stdin().lock().read_line(&mut line)?;
        if line.ends_with('\n') {
            line.pop();
        }
        if line.ends_with('\r') {
            line.pop();
        }
        Ok(line)
    }
    fn read_byte(&mut self) -> std::io::Result<u8> {
        use std::io::Read;
        let mut buf = [0u8];
        std::io::stdin().lock().read_exact(&mut buf)?;
        Ok(buf[0])
    }
    fn write_str(&mut self, s: &str) -> std::io::Result<()> {
        use std::io::Write;
        let mut out = std::io::stdout().lock();
        out.write_all(s.as_bytes())?;
        out.write_all(b"\n")?;
        out.flush()
    }
    fn write_byte(&mut self, b: u8) -> std::io::Result<()> {
        use std::io::Write;
        std::io::stdout().lock().write_all(&[b])
    }
}

/// A value tagged with the source span where it was produced.
#[derive(Debug, Clone)]
pub struct Tagged {
    pub value: Value,
    pub span: std::ops::Range<usize>,
}

/// The ezc engine — a stack machine with dynamically scoped variable bindings.
///
/// All operations modify the stack in place. Variables are stored in a scope
/// chain (innermost scope last). Blocks use dynamic scoping — they see whatever
/// bindings exist at execution time.
/// Embedded standard library modules (shipped with binary).
const EMBEDDED_MODULES: &[(&str, &str)] = &[
    ("std/math.ezc", include_str!("../../../std/math.ezc")),
    ("std/str.ezc", include_str!("../../../std/str.ezc")),
];

pub struct Engine {
    stack: Vec<Tagged>,
    /// Scope chain: index 0 is the outermost (top-level), last is innermost.
    env: Vec<HashMap<String, Value>>,
    /// Interner for deduplicating strings, bins, and big integers.
    pub interner: Interner,
    /// I/O backend.
    io: Box<dyn EzIo>,
    /// Modules already imported (guard against double-import).
    imported: HashSet<String>,
    /// Step counter for execution limits (0 = unlimited).
    steps: u64,
    max_steps: u64,
}

impl Engine {
    pub fn new() -> Self {
        Engine {
            stack: Vec::new(),
            env: vec![HashMap::new()],
            interner: Interner::new(),
            io: Box::new(StdIo),
            imported: HashSet::new(),
            steps: 0,
            max_steps: 0, // 0 = unlimited
        }
    }

    /// Create an engine with a custom I/O backend.
    pub fn with_io(io: Box<dyn EzIo>) -> Self {
        Engine {
            stack: Vec::new(),
            env: vec![HashMap::new()],
            interner: Interner::new(),
            io,
            imported: HashSet::new(),
            steps: 0,
            max_steps: 0,
        }
    }

    /// Set a maximum step limit. 0 = unlimited.
    pub fn set_step_limit(&mut self, limit: u64) {
        self.max_steps = limit;
    }

    // ── Stack access ──────────────────────────────────────────────────────

    /// Returns a view of the current stack values.
    pub fn stack(&self) -> Vec<&Value> {
        self.stack.iter().map(|t| &t.value).collect()
    }

    /// Consumes the engine, returning the final stack values.
    pub fn into_stack(self) -> Vec<Value> {
        self.stack.into_iter().map(|t| t.value).collect()
    }

    /// Replace the stack (used for undo in the REPL).
    pub fn set_stack(&mut self, stack: Vec<Tagged>) {
        self.stack = stack;
    }

    /// Returns a clone of the raw tagged stack (for snapshotting/undo).
    pub fn clone_raw_stack(&self) -> Vec<Tagged> {
        self.stack.clone()
    }

    /// Push a value onto the stack with its source span.
    pub fn push(&mut self, value: Value, span: std::ops::Range<usize>) {
        self.stack.push(Tagged { value, span });
    }

    /// Pop a value from the stack, or `None` if empty.
    pub fn pop_value(&mut self) -> Option<Value> {
        self.stack.pop().map(|t| t.value)
    }

    /// Execute a single expression. Used by the debugger stepper to advance
    /// the program counter one expression at a time.
    pub fn eval_one(&mut self, (expr, span): &Spanned<Expr>) -> Result<(), EvalError> {
        self.eval_expr(expr, span.clone())
    }

    /// Peek at the top of the value stack without consuming it.
    pub fn peek_top(&self) -> Option<&Value> {
        self.stack.last().map(|t| &t.value)
    }

    /// Snapshot the current environment (all scopes, outermost first).
    /// Used by the debugger to display variable bindings.
    pub fn env_snapshot(&self) -> Vec<HashMap<String, Value>> {
        self.env.clone()
    }

    /// Reset the engine: clear stack and all bindings except the top-level scope.
    pub fn reset(&mut self) {
        self.stack.clear();
        self.env.clear();
        self.env.push(HashMap::new());
        self.interner = Interner::new();
        self.imported.clear();
        self.steps = 0;
    }

    // ── Environment ───────────────────────────────────────────────────────

    /// Bind a name in the current (innermost) scope.
    fn bind(&mut self, name: String, value: Value) {
        if let Some(scope) = self.env.last_mut() {
            scope.insert(name, value);
        }
    }

    /// Look up a name, walking from innermost scope outward.
    fn lookup(&self, name: &str) -> Option<&Value> {
        for scope in self.env.iter().rev() {
            if let Some(v) = scope.get(name) {
                return Some(v);
            }
        }
        None
    }

    /// Push a new empty scope (for `{...}` and block execution).
    fn push_scope(&mut self) {
        self.env.push(HashMap::new());
    }

    /// Pop the innermost scope.
    fn pop_scope(&mut self) {
        // Never pop the top-level scope.
        if self.env.len() > 1 {
            self.env.pop();
        }
    }

    /// Create a child engine: empty stack, cloned environment.
    /// Used for list evaluation, map, and filter sub-computations.
    fn child(&self) -> Engine {
        Engine {
            stack: Vec::new(),
            env: self.env.clone(),
            interner: Interner::new(),
            io: Box::new(StdIo),
            imported: self.imported.clone(),
            steps: self.steps,
            max_steps: self.max_steps,
        }
    }

    // ── Evaluation ────────────────────────────────────────────────────────

    /// Evaluate a sequence of expressions.
    pub fn eval(&mut self, program: &[Spanned<Expr>]) -> Result<(), EvalError> {
        for (expr, span) in program {
            trace!(?expr, "eval");
            self.eval_expr(expr, span.clone())?;
        }
        Ok(())
    }

    fn eval_expr(&mut self, expr: &Expr, span: std::ops::Range<usize>) -> Result<(), EvalError> {
        // Step counting for execution limits.
        if self.max_steps > 0 {
            self.steps += 1;
            if self.steps > self.max_steps {
                return Err(EvalError {
                    kind: EvalErrorKind::StepLimitExceeded {
                        limit: self.max_steps,
                    },
                    span: Some(span),
                    labels: vec![],
                });
            }
        }

        match expr {
            Expr::Literal(num) => {
                debug!("push {}", num.type_name());
                let num = match num {
                    Number::Int(n) => self.interner.intern_int(n.as_ref().clone()),
                    other => other.clone(),
                };
                self.stack.push(Tagged {
                    value: Value::Num(num),
                    span: span.clone(),
                });
            }

            Expr::StrLiteral(s) => {
                debug!("push str");
                self.stack.push(Tagged {
                    value: Value::Str(self.interner.intern_str(s)),
                    span: span.clone(),
                });
            }

            Expr::Ident(name) => {
                // No-arg builtins: these don't pop from the stack.
                match name.as_str() {
                    "rl" => {
                        let line = self.io.read_line().map_err(|e| EvalError {
                            kind: EvalErrorKind::IoError(e.to_string()),
                            span: Some(span.clone()),
                            labels: vec![],
                        })?;
                        self.stack.push(Tagged {
                            value: Value::Str(self.interner.intern_str(&line)),
                            span,
                        });
                        return Ok(());
                    }
                    "rb" => {
                        let b = self.io.read_byte().map_err(|e| EvalError {
                            kind: EvalErrorKind::IoError(e.to_string()),
                            span: Some(span.clone()),
                            labels: vec![],
                        })?;
                        self.stack.push(Tagged {
                            value: Value::int(b as i64),
                            span,
                        });
                        return Ok(());
                    }
                    "wl" => {
                        let val = self.pop("wl", &span)?;
                        let text = val.value.print_string();
                        self.io.write_str(&text).map_err(|e| EvalError {
                            kind: EvalErrorKind::IoError(e.to_string()),
                            span: Some(span),
                            labels: vec![],
                        })?;
                        return Ok(());
                    }
                    "wb" => {
                        let val = self.pop("wb", &span)?;
                        match &val.value {
                            Value::Num(n) => {
                                let byte = n.to_f64_lossy() as u8;
                                self.io.write_byte(byte).map_err(|e| EvalError {
                                    kind: EvalErrorKind::IoError(e.to_string()),
                                    span: Some(span),
                                    labels: vec![],
                                })?;
                                return Ok(());
                            }
                            _ => {
                                return Err(EvalError {
                                    kind: EvalErrorKind::TypeMismatch {
                                        op: "wb".into(),
                                        expected: "a number".into(),
                                        found: val.value.type_name().into(),
                                    },
                                    span: Some(span),
                                    labels: vec![],
                                });
                            }
                        }
                    }
                    // Named aliases for symbolic operators
                    "map" => return self.eval_expr(&Expr::Map, span),
                    "fil" => return self.eval_expr(&Expr::Filter, span),
                    "red" => return self.eval_expr(&Expr::Fold, span),
                    "each" => {
                        let block = self.pop("each", &span)?;
                        let collection = self.pop("each", &span)?;
                        match (&collection.value, &block.value) {
                            (Value::List(items), Value::Block(b)) => {
                                let items = items.clone();
                                let body = b.body.clone();
                                for item in items {
                                    self.stack.push(Tagged {
                                        value: item,
                                        span: span.clone(),
                                    });
                                    self.eval(&body)?;
                                }
                            }
                            (Value::Num(n), Value::Block(b)) => {
                                let count = n.to_f64_lossy() as i64;
                                let body = b.body.clone();
                                for i in 0..count {
                                    self.stack.push(Tagged {
                                        value: Value::int(i),
                                        span: span.clone(),
                                    });
                                    self.eval(&body)?;
                                }
                            }
                            _ => {
                                return Err(EvalError {
                                    kind: EvalErrorKind::TypeMismatch {
                                        op: "each".into(),
                                        expected: "a list or number, and a block".into(),
                                        found: format!(
                                            "{} and {}",
                                            collection.value.type_name(),
                                            block.value.type_name()
                                        ),
                                    },
                                    span: Some(span),
                                    labels: vec![],
                                });
                            }
                        }
                        return Ok(());
                    }
                    "loop" => return self.eval_expr(&Expr::Loop, span),
                    "words" => {
                        // Print all defined names (useful for REPL exploration).
                        let mut names: Vec<&String> = Vec::new();
                        for scope in &self.env {
                            names.extend(scope.keys());
                        }
                        names.sort();
                        names.dedup();
                        let list: Vec<Value> = names
                            .into_iter()
                            .map(|n| Value::Str(self.interner.intern_str(n)))
                            .collect();
                        self.stack.push(Tagged {
                            value: Value::List(list),
                            span,
                        });
                        return Ok(());
                    }
                    "import" => {
                        let path_tagged = self.pop("import", &span)?;
                        match &path_tagged.value {
                            Value::Str(s) => {
                                let name = s.0.to_string();

                                // Guard: skip if already imported.
                                if self.imported.contains(&name) {
                                    return Ok(());
                                }
                                self.imported.insert(name.clone());

                                // Check embedded modules first ("math", "str", etc.)
                                let src = EMBEDDED_MODULES
                                    .iter()
                                    .find(|(n, _)| *n == name)
                                    .map(|(_, src)| src.to_string())
                                    .or_else(|| std::fs::read_to_string(&name).ok());

                                let src = src.ok_or_else(|| EvalError {
                                    kind: EvalErrorKind::IoError(format!(
                                        "import \"{name}\": not found"
                                    )),
                                    span: Some(span.clone()),
                                    labels: vec![ErrorLabel {
                                        span: path_tagged.span.clone(),
                                        message: "this path".into(),
                                    }],
                                })?;

                                let ast = crate::lexer::lex(&src)
                                    .map_err(|errs| EvalError {
                                        kind: EvalErrorKind::IoError(format!(
                                            "import \"{name}\": {}",
                                            errs.iter()
                                                .map(|e| e.to_string())
                                                .collect::<Vec<_>>()
                                                .join("; ")
                                        )),
                                        span: Some(span.clone()),
                                        labels: vec![],
                                    })
                                    .and_then(|tokens| {
                                        crate::parser::parse(&tokens, src.len()).map_err(|errs| {
                                            EvalError {
                                                kind: EvalErrorKind::IoError(format!(
                                                    "import \"{name}\": {}",
                                                    errs.iter()
                                                        .map(|e| e.to_string())
                                                        .collect::<Vec<_>>()
                                                        .join("; ")
                                                )),
                                                span: Some(span.clone()),
                                                labels: vec![],
                                            }
                                        })
                                    })?;
                                self.eval(&ast)?;
                            }
                            _ => {
                                return Err(EvalError {
                                    kind: EvalErrorKind::TypeMismatch {
                                        op: "import".into(),
                                        expected: "a string".into(),
                                        found: path_tagged.value.type_name().into(),
                                    },
                                    span: Some(span),
                                    labels: vec![],
                                });
                            }
                        }
                        return Ok(());
                    }
                    _ => {}
                }
                // Check environment before falling through to builtins.
                // Blocks auto-execute; other values push (like $name).
                if let Some(val) = self.lookup(name).cloned() {
                    match val {
                        Value::Block(b) => {
                            self.eval(&b.body)?;
                            return Ok(());
                        }
                        other => {
                            self.stack.push(Tagged { value: other, span });
                            return Ok(());
                        }
                    }
                }
                self.eval_builtin(name, &span)?;
            }

            Expr::Op(op) => {
                let op_name = op.to_string();
                let b = self.pop(&op_name, &span)?;
                let a = self.pop(&op_name, &span)?;
                let result = self.apply_op(*op, a, b, &span)?;
                self.stack.push(Tagged {
                    value: result,
                    span: span.clone(),
                });
            }

            Expr::Execute => {
                let tagged = self.pop("!", &span)?;
                match tagged.value {
                    Value::Block(block) => {
                        debug!("executing block");
                        // Dynamic scope: block sees current env, not definition-time env.
                        self.eval(&block.body)?;
                    }
                    Value::List(items) => {
                        debug!("splatting list ({} items)", items.len());
                        for item in items {
                            self.stack.push(Tagged {
                                value: item,
                                span: span.clone(),
                            });
                        }
                    }
                    Value::Str(s) => {
                        debug!("eval string as code");
                        let src = s.0.to_string();
                        let tokens = crate::lexer::lex(&src).map_err(|errs| EvalError {
                            kind: EvalErrorKind::TypeMismatch {
                                op: "!".into(),
                                expected: "valid ezc code".into(),
                                found: errs
                                    .iter()
                                    .map(|e| e.to_string())
                                    .collect::<Vec<_>>()
                                    .join("; "),
                            },
                            span: Some(span.clone()),
                            labels: vec![],
                        })?;
                        let ast =
                            crate::parser::parse(&tokens, src.len()).map_err(|errs| EvalError {
                                kind: EvalErrorKind::TypeMismatch {
                                    op: "!".into(),
                                    expected: "valid ezc code".into(),
                                    found: errs
                                        .iter()
                                        .map(|e| e.to_string())
                                        .collect::<Vec<_>>()
                                        .join("; "),
                                },
                                span: Some(span.clone()),
                                labels: vec![],
                            })?;
                        self.eval(&ast)?;
                    }
                    other => {
                        return Err(EvalError {
                            kind: EvalErrorKind::TypeMismatch {
                                op: "!".into(),
                                expected: "a block, list, or string".into(),
                                found: other.type_name().into(),
                            },
                            span: Some(span),
                            labels: vec![ErrorLabel {
                                span: tagged.span,
                                message: format!("this is {}", other.type_name()),
                            }],
                        });
                    }
                }
            }

            Expr::Cond => {
                let block = self.pop("?", &span)?;
                let cond = self.pop("?", &span)?;
                if cond.value.is_truthy() {
                    match block.value {
                        Value::Block(b) => self.eval(&b.body)?,
                        other => {
                            // Non-block: just push the value back (conditional push)
                            self.stack.push(Tagged {
                                value: other,
                                span: span.clone(),
                            });
                        }
                    }
                }
            }

            Expr::Ternary => {
                let else_tagged = self.pop("??", &span)?;
                let then_tagged = self.pop("??", &span)?;
                let cond = self.pop("??", &span)?;
                let chosen = if cond.value.is_truthy() {
                    then_tagged
                } else {
                    else_tagged
                };
                match chosen.value {
                    Value::Block(b) => self.eval(&b.body)?,
                    other => {
                        self.stack.push(Tagged {
                            value: other,
                            span: span.clone(),
                        });
                    }
                }
            }

            Expr::Compose => {
                let b = self.pop("|", &span)?;
                let a = self.pop("|", &span)?;
                match (a.value, b.value) {
                    (Value::List(mut a_items), Value::List(b_items)) => {
                        a_items.extend(b_items);
                        self.stack.push(Tagged {
                            value: Value::List(a_items),
                            span: span.clone(),
                        });
                    }
                    (Value::Block(mut a_block), Value::Block(b_block)) => {
                        a_block.body.extend(b_block.body);
                        self.stack.push(Tagged {
                            value: Value::Block(a_block),
                            span: span.clone(),
                        });
                    }
                    (Value::Str(a_str), Value::Str(b_str)) => {
                        let combined = format!("{}{}", a_str.0, b_str.0);
                        self.stack.push(Tagged {
                            value: Value::Str(self.interner.intern_str(&combined)),
                            span: span.clone(),
                        });
                    }
                    (a_val, b_val) => {
                        return Err(EvalError {
                            kind: EvalErrorKind::TypeMismatch {
                                op: "|".into(),
                                expected: "two lists, strings, or blocks".into(),
                                found: format!("{} and {}", a_val.type_name(), b_val.type_name()),
                            },
                            span: Some(span),
                            labels: vec![
                                ErrorLabel {
                                    span: a.span,
                                    message: format!("this is {}", a_val.type_name()),
                                },
                                ErrorLabel {
                                    span: b.span,
                                    message: format!("this is {}", b_val.type_name()),
                                },
                            ],
                        });
                    }
                }
            }

            Expr::Equal => {
                let b = self.pop("==", &span)?;
                let a = self.pop("==", &span)?;
                self.stack.push(Tagged {
                    value: Value::int(if a.value == b.value { 1 } else { 0 }),
                    span: span.clone(),
                });
            }

            Expr::NotEqual => {
                let b = self.pop("!=", &span)?;
                let a = self.pop("!=", &span)?;
                self.stack.push(Tagged {
                    value: Value::int(if a.value != b.value { 1 } else { 0 }),
                    span: span.clone(),
                });
            }

            Expr::Lt => {
                self.compare_nums("<", &span, |o| o.is_lt())?;
            }
            Expr::Gt => {
                self.compare_nums(">", &span, |o| o.is_gt())?;
            }
            Expr::LtEq => {
                self.compare_nums("<=", &span, |o| o.is_le())?;
            }
            Expr::GtEq => {
                self.compare_nums(">=", &span, |o| o.is_ge())?;
            }

            Expr::Swap => {
                let b = self.pop("~", &span)?;
                let a = self.pop("~", &span)?;
                self.stack.push(b);
                self.stack.push(a);
            }

            Expr::Dup => {
                let a = self.pop(",", &span)?;
                self.stack.push(a.clone());
                self.stack.push(a);
            }

            Expr::Drop => {
                self.pop(";", &span)?;
            }

            Expr::Write => {
                let val = self.pop(":", &span)?;
                let text = val.value.print_string();
                self.io.write_str(&text).map_err(|e| EvalError {
                    kind: EvalErrorKind::IoError(e.to_string()),
                    span: Some(span),
                    labels: vec![],
                })?;
            }

            Expr::Read => {
                let line = self.io.read_line().map_err(|e| EvalError {
                    kind: EvalErrorKind::IoError(e.to_string()),
                    span: Some(span.clone()),
                    labels: vec![],
                })?;
                self.stack.push(Tagged {
                    value: Value::Str(self.interner.intern_str(&line)),
                    span,
                });
            }

            Expr::Over => {
                if self.stack.len() < 2 {
                    return Err(EvalError {
                        kind: EvalErrorKind::StackUnderflow {
                            op: "_".into(),
                            expected: 2,
                            found: self.stack.len(),
                        },
                        span: Some(span),
                        labels: vec![],
                    });
                }
                let second = self.stack[self.stack.len() - 2].clone();
                self.stack.push(second);
            }

            // ── Variables ─────────────────────────────────────────────────
            Expr::Bind(name) => {
                let tagged = self.pop(&format!("@{name}"), &span)?;
                debug!(name, "bind");
                self.bind(name.clone(), tagged.value);
            }

            Expr::Recall(name) => {
                let val = self
                    .lookup(name)
                    .ok_or_else(|| EvalError {
                        kind: EvalErrorKind::UndefinedVariable { name: name.clone() },
                        span: Some(span.clone()),
                        labels: vec![],
                    })?
                    .clone();
                debug!(name, "recall");
                self.stack.push(Tagged {
                    value: val,
                    span: span.clone(),
                });
            }

            // ── Containers ────────────────────────────────────────────────
            Expr::Block(body) => {
                debug!("push block ({} exprs)", body.len());
                self.stack.push(Tagged {
                    value: Value::Block(Block { body: body.clone() }),
                    span: span.clone(),
                });
            }

            Expr::List(body) => {
                debug!("evaluating list ({} exprs)", body.len());
                let mut sub = self.child();
                sub.eval(body)?;
                self.stack.push(Tagged {
                    value: Value::List(sub.into_stack()),
                    span: span.clone(),
                });
            }

            Expr::Scope(body) => {
                debug!("entering scope ({} exprs)", body.len());
                self.push_scope();
                let result = self.eval(body);
                self.pop_scope();
                result?;
            }

            // ── Higher-order operators ────────────────────────────────────
            Expr::Map => {
                let block = self.pop("&!", &span)?;
                let list = self.pop("&!", &span)?;
                // Convert numbers to ranges for map.
                let items = match &list.value {
                    Value::List(items) => items.clone(),
                    Value::Num(n) => {
                        let count = n.to_f64_lossy() as i64;
                        (0..count).map(Value::int).collect()
                    }
                    _ => {
                        return Err(EvalError {
                            kind: EvalErrorKind::TypeMismatch {
                                op: "&!".into(),
                                expected: "a list or number, and a block".into(),
                                found: format!(
                                    "{} and {}",
                                    list.value.type_name(),
                                    block.value.type_name()
                                ),
                            },
                            span: Some(span),
                            labels: vec![
                                ErrorLabel {
                                    span: list.span,
                                    message: format!("this is {}", list.value.type_name()),
                                },
                                ErrorLabel {
                                    span: block.span,
                                    message: format!("this is {}", block.value.type_name()),
                                },
                            ],
                        });
                    }
                };
                match &block.value {
                    Value::Block(b) => {
                        let mut result = Vec::with_capacity(items.len());
                        for item in items {
                            let mut sub = self.child();
                            sub.stack.push(Tagged {
                                value: item,
                                span: span.clone(),
                            });
                            sub.eval(&b.body)?;
                            match sub.stack.pop() {
                                Some(t) => result.push(t.value),
                                None => {
                                    return Err(EvalError {
                                        kind: EvalErrorKind::StackUnderflow {
                                            op: "&! (map body produced no value)".into(),
                                            expected: 1,
                                            found: 0,
                                        },
                                        span: Some(span),
                                        labels: vec![],
                                    });
                                }
                            }
                        }
                        self.stack.push(Tagged {
                            value: Value::List(result),
                            span: span.clone(),
                        });
                    }
                    _ => {
                        return Err(EvalError {
                            kind: EvalErrorKind::TypeMismatch {
                                op: "&!".into(),
                                expected: "a block".into(),
                                found: block.value.type_name().into(),
                            },
                            span: Some(span),
                            labels: vec![ErrorLabel {
                                span: block.span,
                                message: format!("this is {}", block.value.type_name()),
                            }],
                        });
                    }
                }
            }

            Expr::Filter => {
                let block = self.pop("&?", &span)?;
                let list = self.pop("&?", &span)?;
                let items: Vec<Value> = match &list.value {
                    Value::List(items) => items.clone(),
                    Value::Num(n) => {
                        let count = n.to_f64_lossy() as i64;
                        (0..count).map(Value::int).collect()
                    }
                    _ => {
                        return Err(EvalError {
                            kind: EvalErrorKind::TypeMismatch {
                                op: "&?".into(),
                                expected: "a list or number, and a block".into(),
                                found: format!(
                                    "{} and {}",
                                    list.value.type_name(),
                                    block.value.type_name()
                                ),
                            },
                            span: Some(span),
                            labels: vec![],
                        });
                    }
                };
                match &block.value {
                    Value::Block(b) => {
                        let mut result = Vec::new();
                        for item in items {
                            let mut sub = self.child();
                            sub.stack.push(Tagged {
                                value: item.clone(),
                                span: span.clone(),
                            });
                            sub.eval(&b.body)?;
                            match sub.stack.pop() {
                                Some(t) if t.value.is_truthy() => result.push(item),
                                _ => {}
                            }
                        }
                        self.stack.push(Tagged {
                            value: Value::List(result),
                            span: span.clone(),
                        });
                    }
                    _ => {
                        return Err(EvalError {
                            kind: EvalErrorKind::TypeMismatch {
                                op: "&?".into(),
                                expected: "a block".into(),
                                found: block.value.type_name().into(),
                            },
                            span: Some(span),
                            labels: vec![],
                        });
                    }
                }
            }

            Expr::Loop => {
                let body = self.pop("&", &span)?;
                let cond = self.pop("&", &span)?;
                match (cond.value, body.value) {
                    (Value::Block(cond_block), Value::Block(body_block)) => loop {
                        self.eval(&cond_block.body)?;
                        let flag = self.pop("& (condition result)", &span)?;
                        if !flag.value.is_truthy() {
                            break;
                        }
                        self.eval(&body_block.body)?;
                    },
                    (c, b) => {
                        return Err(EvalError {
                            kind: EvalErrorKind::TypeMismatch {
                                op: "&".into(),
                                expected: "two blocks".into(),
                                found: format!("{} and {}", c.type_name(), b.type_name()),
                            },
                            span: Some(span),
                            labels: vec![
                                ErrorLabel {
                                    span: cond.span,
                                    message: format!("this is {}", c.type_name()),
                                },
                                ErrorLabel {
                                    span: body.span,
                                    message: format!("this is {}", b.type_name()),
                                },
                            ],
                        });
                    }
                }
            }

            Expr::Fold => {
                let block_tagged = self.pop("&/", &span)?;
                let init = self.pop("&/", &span)?;
                let list_tagged = self.pop("&/", &span)?;
                match (&list_tagged.value, &block_tagged.value) {
                    (Value::List(items), Value::Block(b)) => {
                        let items = items.clone();
                        let body = b.body.clone();
                        self.stack.push(init);
                        for item in items {
                            self.stack.push(Tagged {
                                value: item,
                                span: span.clone(),
                            });
                            self.eval(&body)?;
                        }
                    }
                    _ => {
                        return Err(EvalError {
                            kind: EvalErrorKind::TypeMismatch {
                                op: "&/".into(),
                                expected: "a list and a block".into(),
                                found: format!(
                                    "{} and {}",
                                    list_tagged.value.type_name(),
                                    block_tagged.value.type_name()
                                ),
                            },
                            span: Some(span),
                            labels: vec![
                                ErrorLabel {
                                    span: list_tagged.span,
                                    message: format!("this is {}", list_tagged.value.type_name()),
                                },
                                ErrorLabel {
                                    span: block_tagged.span,
                                    message: format!("this is {}", block_tagged.value.type_name()),
                                },
                            ],
                        });
                    }
                }
            }
        }

        trace!(stack_depth = self.stack.len(), "after eval");
        Ok(())
    }

    /// Evaluate a built-in identifier (type constructor / conversion function).
    fn eval_builtin(&mut self, name: &str, span: &std::ops::Range<usize>) -> Result<(), EvalError> {
        use crate::number::Number;

        let tagged = self.pop(name, span)?;
        let val = tagged.value;
        let result = match name {
            // ── Numeric constructors ──────────────────────────────────────
            "int" => match val {
                Value::Num(n) => match n {
                    Number::Int(_) => Value::Num(n),
                    _ => {
                        let bi = n.to_bigint_lossy();
                        Value::Num(self.interner.intern_int(bi))
                    }
                },
                _ => return Err(self.cast_error(name, &val, span)),
            },
            "u8" => Value::Num(self.cast_uint::<u8>(&val, span, Number::U8)?),
            "u16" => Value::Num(self.cast_uint::<u16>(&val, span, Number::U16)?),
            "u32" => Value::Num(self.cast_uint::<u32>(&val, span, Number::U32)?),
            "u64" => Value::Num(self.cast_uint::<u64>(&val, span, Number::U64)?),
            "u128" => Value::Num(self.cast_uint::<u128>(&val, span, Number::U128)?),
            "i8" => Value::Num(self.cast_sint::<i8>(&val, span, Number::I8)?),
            "i16" => Value::Num(self.cast_sint::<i16>(&val, span, Number::I16)?),
            "i32" => Value::Num(self.cast_sint::<i32>(&val, span, Number::I32)?),
            "i64" => Value::Num(self.cast_sint::<i64>(&val, span, Number::I64)?),
            "i128" => Value::Num(self.cast_sint::<i128>(&val, span, Number::I128)?),
            "f16" => match &val {
                Value::Num(n) => Value::Num(Number::F16(half::f16::from_f64(n.to_f64_lossy()))),
                _ => return Err(self.cast_error(name, &val, span)),
            },
            "f32" => match &val {
                Value::Num(n) => Value::Num(Number::F32(n.to_f64_lossy() as f32)),
                _ => return Err(self.cast_error(name, &val, span)),
            },
            "f64" => match &val {
                Value::Num(n) => Value::Num(Number::F64(n.to_f64_lossy())),
                _ => return Err(self.cast_error(name, &val, span)),
            },
            // ── String / binary constructors ──────────────────────────────
            "str" => {
                let s = match &val {
                    Value::Num(n) => n.to_string(),
                    Value::Str(_) => {
                        self.stack.push(Tagged {
                            value: val,
                            span: span.clone(),
                        });
                        return Ok(());
                    }
                    _ => return Err(self.cast_error(name, &val, span)),
                };
                Value::Str(self.interner.intern_str(&s))
            }
            "bin" => match &val {
                Value::Str(s) => Value::Bin(self.interner.intern_bin(s.0.as_bytes())),
                Value::Bin(_) => val,
                _ => return Err(self.cast_error(name, &val, span)),
            },

            // ── Collection operations ─────────────────────────────────────
            "len" => match &val {
                Value::List(items) => Value::int(items.len() as i64),
                Value::Str(s) => Value::int(s.0.len() as i64),
                Value::Bin(b) => Value::int(b.0.len() as i64),
                _ => return Err(self.cast_error(name, &val, span)),
            },
            "tl" => match val {
                Value::List(mut items) => {
                    if items.is_empty() {
                        return Err(EvalError {
                            kind: EvalErrorKind::StackUnderflow {
                                op: "tl".into(),
                                expected: 1,
                                found: 0,
                            },
                            span: Some(span.clone()),
                            labels: vec![],
                        });
                    }
                    items.remove(0);
                    Value::List(items)
                }
                _ => return Err(self.cast_error(name, &val, span)),
            },
            "rev" => match val {
                Value::List(mut items) => {
                    items.reverse();
                    Value::List(items)
                }
                Value::Str(s) => {
                    let reversed: String = s.0.chars().rev().collect();
                    Value::Str(self.interner.intern_str(&reversed))
                }
                _ => return Err(self.cast_error(name, &val, span)),
            },
            "srt" => match val {
                Value::List(mut items) => {
                    items.sort_by_key(|a| a.to_string());
                    Value::List(items)
                }
                _ => return Err(self.cast_error(name, &val, span)),
            },
            "take" => {
                // list/str n take → first n elements/chars
                let n = match &val {
                    Value::Num(n) => {
                        let n_val = n.to_f64_lossy();
                        if n_val < 0.0 {
                            return Err(self.cast_error(name, &val, span));
                        }
                        n_val as usize
                    }
                    _ => return Err(self.cast_error(name, &val, span)),
                };
                let coll = self.pop("take", span)?;
                match coll.value {
                    Value::List(items) => Value::List(items.into_iter().take(n).collect()),
                    Value::Str(s) => {
                        let taken: String = s.0.chars().take(n).collect();
                        Value::Str(self.interner.intern_str(&taken))
                    }
                    _ => return Err(self.cast_error(name, &coll.value, span)),
                }
            }
            "skip" => {
                // list/str n skip → everything after first n
                let n = match &val {
                    Value::Num(n) => {
                        let n_val = n.to_f64_lossy();
                        if n_val < 0.0 {
                            return Err(self.cast_error(name, &val, span));
                        }
                        n_val as usize
                    }
                    _ => return Err(self.cast_error(name, &val, span)),
                };
                let coll = self.pop("skip", span)?;
                match coll.value {
                    Value::List(items) => Value::List(items.into_iter().skip(n).collect()),
                    Value::Str(s) => {
                        let skipped: String = s.0.chars().skip(n).collect();
                        Value::Str(self.interner.intern_str(&skipped))
                    }
                    _ => return Err(self.cast_error(name, &coll.value, span)),
                }
            }
            "zip" => {
                // list1 list2 zip → [[a1 b1] [a2 b2] ...]
                let list2_tagged = self.pop("zip", span)?;
                match (&val, &list2_tagged.value) {
                    (Value::List(b), Value::List(a)) => {
                        let pairs: Vec<Value> = a
                            .iter()
                            .zip(b.iter())
                            .map(|(x, y)| Value::List(vec![x.clone(), y.clone()]))
                            .collect();
                        Value::List(pairs)
                    }
                    _ => return Err(self.cast_error(name, &val, span)),
                }
            }
            "typeof" => {
                let type_name = val.type_name();
                Value::Str(self.interner.intern_str(type_name))
            }
            "range" => {
                // start end range → [start start+1 ... end-1]
                let end = match &val {
                    Value::Num(n) => n.to_f64_lossy() as i64,
                    _ => return Err(self.cast_error(name, &val, span)),
                };
                let start_tagged = self.pop("range", span)?;
                let start = match &start_tagged.value {
                    Value::Num(n) => n.to_f64_lossy() as i64,
                    _ => return Err(self.cast_error(name, &start_tagged.value, span)),
                };
                let items: Vec<Value> = (start..end).map(Value::int).collect();
                Value::List(items)
            }
            "nth" => {
                // `nth` pops index (already popped as `val`), then pops list.
                let index = match &val {
                    Value::Num(n) => n.to_f64_lossy() as usize,
                    _ => return Err(self.cast_error(name, &val, span)),
                };
                let list_tagged = self.pop("nth", span)?;
                match list_tagged.value {
                    Value::List(items) => {
                        if index >= items.len() {
                            return Err(EvalError {
                                kind: EvalErrorKind::TypeMismatch {
                                    op: "nth".into(),
                                    expected: format!("index < {} (list length)", items.len()),
                                    found: format!("index {index}"),
                                },
                                span: Some(span.clone()),
                                labels: vec![],
                            });
                        }
                        items.into_iter().nth(index).unwrap()
                    }
                    _ => return Err(self.cast_error(name, &list_tagged.value, span)),
                }
            }

            // ── String split/join ─────────────────────────────────────────
            "cut" => {
                // "a,b,c" "," cut → ["a" "b" "c"]
                // val = delimiter (popped first), need to pop the string
                let str_tagged = self.pop("cut", span)?;
                match (&str_tagged.value, &val) {
                    (Value::Str(s), Value::Str(delim)) => {
                        let parts: Vec<Value> =
                            s.0.split(delim.0.as_ref())
                                .map(|p| Value::Str(self.interner.intern_str(p)))
                                .collect();
                        Value::List(parts)
                    }
                    _ => return Err(self.cast_error(name, &str_tagged.value, span)),
                }
            }
            "cat" => {
                // ["a" "b" "c"] "," cat → "a,b,c"
                // val = delimiter (popped first), need to pop the list
                let list_tagged = self.pop("cat", span)?;
                match (&list_tagged.value, &val) {
                    (Value::List(items), Value::Str(delim)) => {
                        let joined: String = items
                            .iter()
                            .map(|v| match v {
                                Value::Str(s) => s.0.to_string(),
                                other => other.to_string(),
                            })
                            .collect::<Vec<_>>()
                            .join(delim.0.as_ref());
                        Value::Str(self.interner.intern_str(&joined))
                    }
                    _ => return Err(self.cast_error(name, &list_tagged.value, span)),
                }
            }

            _ => {
                return Err(EvalError {
                    kind: EvalErrorKind::UndefinedVariable {
                        name: name.to_string(),
                    },
                    span: Some(span.clone()),
                    labels: vec![],
                });
            }
        };
        self.stack.push(Tagged {
            value: result,
            span: span.clone(),
        });
        Ok(())
    }

    fn cast_error(&self, target: &str, val: &Value, span: &std::ops::Range<usize>) -> EvalError {
        EvalError {
            kind: EvalErrorKind::TypeMismatch {
                op: target.into(),
                expected: "a number".into(),
                found: val.type_name().into(),
            },
            span: Some(span.clone()),
            labels: vec![],
        }
    }

    fn cast_uint<T>(
        &self,
        val: &Value,
        span: &std::ops::Range<usize>,
        wrap: fn(T) -> Number,
    ) -> Result<Number, EvalError>
    where
        T: TryFrom<u64> + TryFrom<u128>,
        u64: TryFrom<T>,
    {
        use num_traits::ToPrimitive;
        match val {
            Value::Num(n) => {
                let big = n.to_bigint_lossy();
                let v: u128 = big.to_u128().ok_or_else(|| EvalError {
                    kind: EvalErrorKind::TypeMismatch {
                        op: std::any::type_name::<T>().into(),
                        expected: "value in range".into(),
                        found: "out of range".into(),
                    },
                    span: Some(span.clone()),
                    labels: vec![],
                })?;
                let t = T::try_from(v).map_err(|_| EvalError {
                    kind: EvalErrorKind::TypeMismatch {
                        op: std::any::type_name::<T>().into(),
                        expected: "value in range".into(),
                        found: "out of range".into(),
                    },
                    span: Some(span.clone()),
                    labels: vec![],
                })?;
                Ok(wrap(t))
            }
            _ => Err(self.cast_error(std::any::type_name::<T>(), val, span)),
        }
    }

    fn cast_sint<T>(
        &self,
        val: &Value,
        span: &std::ops::Range<usize>,
        wrap: fn(T) -> Number,
    ) -> Result<Number, EvalError>
    where
        T: TryFrom<i64> + TryFrom<i128>,
    {
        use num_traits::ToPrimitive;
        match val {
            Value::Num(n) => {
                let big = n.to_bigint_lossy();
                let v: i128 = big.to_i128().ok_or_else(|| EvalError {
                    kind: EvalErrorKind::TypeMismatch {
                        op: std::any::type_name::<T>().into(),
                        expected: "value in range".into(),
                        found: "out of range".into(),
                    },
                    span: Some(span.clone()),
                    labels: vec![],
                })?;
                let t = T::try_from(v).map_err(|_| EvalError {
                    kind: EvalErrorKind::TypeMismatch {
                        op: std::any::type_name::<T>().into(),
                        expected: "value in range".into(),
                        found: "out of range".into(),
                    },
                    span: Some(span.clone()),
                    labels: vec![],
                })?;
                Ok(wrap(t))
            }
            _ => Err(self.cast_error(std::any::type_name::<T>(), val, span)),
        }
    }

    /// Compare two numeric values.
    fn compare_nums(
        &mut self,
        op: &str,
        span: &std::ops::Range<usize>,
        f: fn(std::cmp::Ordering) -> bool,
    ) -> Result<(), EvalError> {
        let b = self.pop(op, span)?;
        let a = self.pop(op, span)?;
        match (&a.value, &b.value) {
            (Value::Num(an), Value::Num(bn)) => {
                let ord = number::compare(an, bn).map_err(|e| self.arith_to_eval(e, span))?;
                self.stack.push(Tagged {
                    value: Value::int(if f(ord) { 1 } else { 0 }),
                    span: span.clone(),
                });
                Ok(())
            }
            (Value::Str(a_str), Value::Str(b_str)) => {
                let ord = a_str.0.cmp(&b_str.0);
                self.stack.push(Tagged {
                    value: Value::int(if f(ord) { 1 } else { 0 }),
                    span: span.clone(),
                });
                Ok(())
            }
            _ => Err(EvalError {
                kind: EvalErrorKind::TypeMismatch {
                    op: op.into(),
                    expected: "two comparable values".into(),
                    found: format!("{} and {}", a.value.type_name(), b.value.type_name()),
                },
                span: Some(span.clone()),
                labels: vec![
                    ErrorLabel {
                        span: a.span,
                        message: format!("this is {}", a.value.type_name()),
                    },
                    ErrorLabel {
                        span: b.span,
                        message: format!("this is {}", b.value.type_name()),
                    },
                ],
            }),
        }
    }

    fn apply_op(
        &self,
        op: Op,
        a: Tagged,
        b: Tagged,
        span: &std::ops::Range<usize>,
    ) -> Result<Value, EvalError> {
        match (&a.value, &b.value) {
            (Value::Num(an), Value::Num(bn)) => {
                let arith_op = match op {
                    Op::Add => ArithOp::Add,
                    Op::Sub => ArithOp::Sub,
                    Op::Mul => ArithOp::Mul,
                    Op::Div => ArithOp::Div,
                    Op::Mod => ArithOp::Mod,
                    Op::Pow => ArithOp::Pow,
                };
                let result = number::arith(an.clone(), bn.clone(), arith_op)
                    .map_err(|e| self.arith_to_eval(e, span))?;
                Ok(Value::Num(result))
            }
            _ => Err(EvalError {
                kind: EvalErrorKind::TypeMismatch {
                    op: op.to_string(),
                    expected: "two numbers".into(),
                    found: format!("{} and {}", a.value.type_name(), b.value.type_name()),
                },
                span: Some(span.clone()),
                labels: vec![
                    ErrorLabel {
                        span: a.span,
                        message: format!("this is {}", a.value.type_name()),
                    },
                    ErrorLabel {
                        span: b.span,
                        message: format!("this is {}", b.value.type_name()),
                    },
                ],
            }),
        }
    }

    /// Convert a number::ArithError into an EvalError.
    fn arith_to_eval(&self, err: number::ArithError, span: &std::ops::Range<usize>) -> EvalError {
        let kind = match err {
            number::ArithError::Overflow => EvalErrorKind::TypeMismatch {
                op: "arithmetic".into(),
                expected: "result within range".into(),
                found: "overflow".into(),
            },
            number::ArithError::DivisionByZero => EvalErrorKind::DivisionByZero,
            number::ArithError::IncompatibleTypes { left, right } => EvalErrorKind::TypeMismatch {
                op: "arithmetic".into(),
                expected: "same numeric family (e.g. both unsigned, or both signed)".into(),
                found: format!("{left} and {right}"),
            },
        };
        EvalError {
            kind,
            span: Some(span.clone()),
            labels: vec![],
        }
    }

    fn pop(&mut self, op: &str, span: &std::ops::Range<usize>) -> Result<Tagged, EvalError> {
        self.stack.pop().ok_or_else(|| EvalError {
            kind: EvalErrorKind::StackUnderflow {
                op: op.into(),
                expected: 1,
                found: 0,
            },
            span: Some(span.clone()),
            labels: vec![],
        })
    }

    /// Pop cond and body blocks for `&` (loop), matching [`Expr::Loop`] semantics.
    /// Used by the debugger stepper so loop bodies step expression-by-expression.
    pub(crate) fn pop_loop_blocks(
        &mut self,
        span: std::ops::Range<usize>,
    ) -> Result<(crate::types::Block, crate::types::Block), EvalError> {
        use crate::error::ErrorLabel;
        use crate::types::Value;

        let body = self.pop("&", &span)?;
        let cond = self.pop("&", &span)?;
        let cond_span = cond.span.clone();
        let body_span = body.span.clone();
        match (cond.value, body.value) {
            (Value::Block(cb), Value::Block(bb)) => Ok((cb, bb)),
            (c, b) => Err(EvalError {
                kind: EvalErrorKind::TypeMismatch {
                    op: "&".into(),
                    expected: "two blocks".into(),
                    found: format!("{} and {}", c.type_name(), b.type_name()),
                },
                span: Some(span),
                labels: vec![
                    ErrorLabel {
                        span: cond_span,
                        message: format!("this is {}", c.type_name()),
                    },
                    ErrorLabel {
                        span: body_span,
                        message: format!("this is {}", b.type_name()),
                    },
                ],
            }),
        }
    }
}

impl Default for Engine {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::types::EzStr;
    use crate::{lexer, parser};
    use num_bigint::BigInt;

    fn run(src: &str) -> Vec<Value> {
        let tokens = lexer::lex(src).expect("lex failed");
        let ast = parser::parse(&tokens, src.len()).expect("parse failed");
        let mut engine = Engine::new();
        engine.eval(&ast).expect("eval failed");
        engine.into_stack()
    }

    fn run_err(src: &str) -> EvalError {
        let tokens = lexer::lex(src).expect("lex failed");
        let ast = parser::parse(&tokens, src.len()).expect("parse failed");
        let mut engine = Engine::new();
        engine.eval(&ast).expect_err("expected eval error")
    }

    // ── Arithmetic ────────────────────────────────────────────────────────

    #[test]
    fn push_integer() {
        assert_eq!(run("42"), vec![Value::int(42)]);
    }

    #[test]
    fn addition() {
        assert_eq!(run("3 4 +"), vec![Value::int(7)]);
    }

    #[test]
    fn subtraction() {
        assert_eq!(run("10 3 -"), vec![Value::int(7)]);
    }

    #[test]
    fn multiplication() {
        assert_eq!(run("6 7 *"), vec![Value::int(42)]);
    }

    #[test]
    fn division() {
        assert_eq!(run("15 4 /"), vec![Value::int(3)]);
    }

    #[test]
    fn modulo() {
        assert_eq!(run("15 4 %"), vec![Value::int(3)]);
    }

    #[test]
    fn power() {
        assert_eq!(run("2 10 ^"), vec![Value::int(1024)]);
    }

    #[test]
    fn chained_arithmetic() {
        assert_eq!(run("3 4 + 2 *"), vec![Value::int(14)]);
    }

    // ── Stack ops ─────────────────────────────────────────────────────────

    #[test]
    fn swap() {
        assert_eq!(run("1 2 ~"), vec![Value::int(2), Value::int(1)]);
    }

    #[test]
    fn dup() {
        assert_eq!(run("5 ,"), vec![Value::int(5), Value::int(5)]);
    }

    #[test]
    fn dup_square() {
        assert_eq!(run("7 , *"), vec![Value::int(49)]);
    }

    #[test]
    fn over() {
        assert_eq!(
            run("1 2 _"),
            vec![Value::int(1), Value::int(2), Value::int(1)]
        );
    }

    // ── Blocks & control flow ─────────────────────────────────────────────

    #[test]
    fn block_execute() {
        assert_eq!(run("3 (4 +) !"), vec![Value::int(7)]);
    }

    #[test]
    fn conditional_truthy() {
        assert_eq!(run("1 (5) ?"), vec![Value::int(5)]);
    }

    #[test]
    fn conditional_falsy() {
        assert_eq!(run("0 (5) ?"), vec![]);
    }

    #[test]
    fn ternary_truthy() {
        assert_eq!(run("1 (20) (10) ??"), vec![Value::int(20)]);
    }

    #[test]
    fn ternary_falsy() {
        assert_eq!(run("0 (20) (10) ??"), vec![Value::int(10)]);
    }

    // ── Lists ─────────────────────────────────────────────────────────────

    #[test]
    fn list_creation() {
        assert_eq!(
            run("[1 2 3]"),
            vec![Value::List(vec![
                Value::int(1),
                Value::int(2),
                Value::int(3),
            ])]
        );
    }

    #[test]
    fn list_with_arithmetic() {
        assert_eq!(run("[3 4 +]"), vec![Value::List(vec![Value::int(7)])]);
    }

    #[test]
    fn compose_lists() {
        assert_eq!(
            run("[1 2] [3 4] |"),
            vec![Value::List(vec![
                Value::int(1),
                Value::int(2),
                Value::int(3),
                Value::int(4),
            ])]
        );
    }

    #[test]
    fn compose_blocks() {
        // (1 +) (2 *) | → block that does 1+ then 2*
        assert_eq!(run("3 (1 +) (2 *) | !"), vec![Value::int(8)]);
    }

    #[test]
    fn compose_strings() {
        assert_eq!(
            run(r#""hello" " world" |"#),
            vec![Value::Str(EzStr::new("hello world"))]
        );
    }

    // ── Comparison ────────────────────────────────────────────────────────

    #[test]
    fn equality() {
        assert_eq!(run("3 3 =="), vec![Value::int(1)]);
        assert_eq!(run("3 4 =="), vec![Value::int(0)]);
    }

    #[test]
    fn not_equal() {
        assert_eq!(run("3 4 !="), vec![Value::int(1)]);
        assert_eq!(run("3 3 !="), vec![Value::int(0)]);
    }

    #[test]
    fn less_than() {
        assert_eq!(run("3 4 <"), vec![Value::int(1)]);
        assert_eq!(run("4 3 <"), vec![Value::int(0)]);
        assert_eq!(run("3 3 <"), vec![Value::int(0)]);
    }

    #[test]
    fn greater_than() {
        assert_eq!(run("4 3 >"), vec![Value::int(1)]);
        assert_eq!(run("3 4 >"), vec![Value::int(0)]);
    }

    #[test]
    fn less_than_or_equal() {
        assert_eq!(run("3 4 <="), vec![Value::int(1)]);
        assert_eq!(run("3 3 <="), vec![Value::int(1)]);
        assert_eq!(run("4 3 <="), vec![Value::int(0)]);
    }

    #[test]
    fn greater_than_or_equal() {
        assert_eq!(run("4 3 >="), vec![Value::int(1)]);
        assert_eq!(run("3 3 >="), vec![Value::int(1)]);
        assert_eq!(run("3 4 >="), vec![Value::int(0)]);
    }

    // ── Map, filter, loop ─────────────────────────────────────────────────

    #[test]
    fn map() {
        assert_eq!(
            run("[1 2 3] (1 +) &!"),
            vec![Value::List(vec![
                Value::int(2),
                Value::int(3),
                Value::int(4),
            ])]
        );
    }

    #[test]
    fn filter() {
        assert_eq!(
            run("[1 2 3] (1 ==) &?"),
            vec![Value::List(vec![Value::int(1)])]
        );
    }

    #[test]
    fn filter_with_comparison() {
        assert_eq!(
            run("[1 2 3 4 5] (3 >) &?"),
            vec![Value::List(vec![Value::int(4), Value::int(5)])]
        );
    }

    #[test]
    fn loop_countdown() {
        assert_eq!(run("5 (, 0 !=) (1 -) &"), vec![Value::int(0)]);
    }

    #[test]
    fn loop_zero_iters() {
        assert_eq!(run("0 (, 0 !=) (1 +) &"), vec![Value::int(0)]);
    }

    // ── Variables ─────────────────────────────────────────────────────────

    #[test]
    fn bind_recall() {
        assert_eq!(run("5 @x $x $x +"), vec![Value::int(10)]);
    }

    #[test]
    fn named_function() {
        assert_eq!(run("(, *) @square 5 $square !"), vec![Value::int(25)]);
    }

    #[test]
    fn scope_isolation() {
        // Binding inside {} should not leak out.
        assert_eq!(run("{ 10 @temp $temp 2 * }"), vec![Value::int(20)]);
        let err = run_err("{ 10 @temp } $temp");
        assert!(matches!(err.kind, EvalErrorKind::UndefinedVariable { .. }));
    }

    #[test]
    fn scope_shadowing() {
        // Inner scope shadows outer, outer unaffected after.
        assert_eq!(
            run("1 @x { 2 @x $x } $x"),
            vec![Value::int(2), Value::int(1)]
        );
    }

    #[test]
    fn dynamic_scope() {
        // Blocks use dynamic scope — they see bindings at execution time.
        assert_eq!(run("5 @x ($x 1 +) @inc $inc !"), vec![Value::int(6)]);
    }

    #[test]
    fn variables_in_list() {
        assert_eq!(
            run("3 @x [$x $x $x]"),
            vec![Value::List(vec![
                Value::int(3),
                Value::int(3),
                Value::int(3),
            ])]
        );
    }

    #[test]
    fn variables_in_map() {
        assert_eq!(
            run("2 @n [1 2 3] ($n *) &!"),
            vec![Value::List(vec![
                Value::int(2),
                Value::int(4),
                Value::int(6),
            ])]
        );
    }

    #[test]
    fn undefined_variable() {
        let err = run_err("$nope");
        assert!(matches!(
            err.kind,
            EvalErrorKind::UndefinedVariable { ref name } if name == "nope"
        ));
    }

    #[test]
    fn named_block_forward_ref() {
        // Block defined before binding is visible via dynamic scope.
        // Define @double, then use it.
        assert_eq!(run("(2 *) @double 7 $double !"), vec![Value::int(14)]);
    }

    // ── Errors ────────────────────────────────────────────────────────────

    #[test]
    fn division_by_zero() {
        let err = run_err("1 0 /");
        assert!(matches!(err.kind, EvalErrorKind::DivisionByZero));
    }

    #[test]
    fn stack_underflow() {
        let err = run_err("+");
        assert!(matches!(err.kind, EvalErrorKind::StackUnderflow { .. }));
    }

    #[test]
    fn type_mismatch_execute() {
        let err = run_err("42 !");
        assert!(matches!(err.kind, EvalErrorKind::TypeMismatch { .. }));
    }

    #[test]
    fn big_integers() {
        let result = run("2 100 ^");
        assert_eq!(result.len(), 1);
        let expected: BigInt = BigInt::from(2).pow(100);
        assert_eq!(result[0], Value::int(expected));
    }

    #[test]
    fn multiple_values_on_stack() {
        assert_eq!(
            run("1 2 3"),
            vec![Value::int(1), Value::int(2), Value::int(3)]
        );
    }
}
