use num_bigint::BigInt;
use num_traits::{ToPrimitive, Zero};
use tracing::{debug, trace};

use crate::ast::{Expr, Spanned};
use crate::error::{EvalError, EvalErrorKind};
use crate::token::Op;
use crate::types::{Block, Value};

/// The ezc stack machine.
///
/// Evaluates a sequence of AST expressions against a value stack.
/// All operations modify the stack in place.
pub struct Machine {
    stack: Vec<Value>,
}

impl Machine {
    pub fn new() -> Self {
        Machine { stack: Vec::new() }
    }

    /// Returns a view of the current stack.
    pub fn stack(&self) -> &[Value] {
        &self.stack
    }

    /// Consumes the machine, returning the final stack.
    pub fn into_stack(self) -> Vec<Value> {
        self.stack
    }

    /// Replace the stack with the given values (used for undo).
    pub fn set_stack(&mut self, stack: Vec<Value>) {
        self.stack = stack;
    }

    /// Evaluate a sequence of expressions.
    pub fn eval(&mut self, program: &[Spanned<Expr>]) -> Result<(), EvalError> {
        for (expr, span) in program {
            trace!(?expr, "eval");
            self.eval_expr(expr, span.clone())?;
        }
        Ok(())
    }

    fn eval_expr(&mut self, expr: &Expr, span: std::ops::Range<usize>) -> Result<(), EvalError> {
        match expr {
            Expr::Literal(s) => {
                let n: BigInt = s.parse().expect("lexer guarantees valid integers");
                debug!(value = %n, "push int");
                self.stack.push(Value::Int(n));
            }

            Expr::Op(op) => {
                let b = self.pop("binary op", &span)?;
                let a = self.pop("binary op", &span)?;
                let result = self.apply_op(*op, a, b, &span)?;
                self.stack.push(result);
            }

            Expr::Execute => {
                let val = self.pop("!", &span)?;
                match val {
                    Value::Block(block) => {
                        debug!("executing block");
                        self.eval(&block.body)?;
                    }
                    other => {
                        return Err(EvalError {
                            kind: EvalErrorKind::TypeMismatch {
                                op: "!".into(),
                                expected: "block".into(),
                                found: other.type_name().into(),
                            },
                            span: Some(span),
                        });
                    }
                }
            }

            Expr::Cond => {
                // `a ?` — if a is falsy, pop another value.
                let condition = self.pop("?", &span)?;
                if !condition.is_truthy() {
                    let _ = self.pop("? (discard)", &span)?;
                }
            }

            Expr::Ternary => {
                // `a b c ??` — if c is truthy, keep b (discard a); if falsy, keep a (discard b).
                let condition = self.pop("??", &span)?;
                let truthy_val = self.pop("??", &span)?;
                let falsy_val = self.pop("??", &span)?;
                if condition.is_truthy() {
                    self.stack.push(truthy_val);
                } else {
                    self.stack.push(falsy_val);
                }
            }

            Expr::Compose => {
                // `[a] [b] |` — concatenate two lists.
                let b = self.pop("|", &span)?;
                let a = self.pop("|", &span)?;
                match (a, b) {
                    (Value::List(mut a_items), Value::List(b_items)) => {
                        a_items.extend(b_items);
                        self.stack.push(Value::List(a_items));
                    }
                    (a, b) => {
                        return Err(EvalError {
                            kind: EvalErrorKind::TypeMismatch {
                                op: "|".into(),
                                expected: "list, list".into(),
                                found: format!("{}, {}", a.type_name(), b.type_name()),
                            },
                            span: Some(span),
                        });
                    }
                }
            }

            Expr::Equal => {
                let b = self.pop("==", &span)?;
                let a = self.pop("==", &span)?;
                self.stack.push(Value::Bool(a == b));
            }

            Expr::NotEqual => {
                let b = self.pop("!=", &span)?;
                let a = self.pop("!=", &span)?;
                self.stack.push(Value::Bool(a != b));
            }

            Expr::Lt => {
                let b = self.pop("<", &span)?;
                let a = self.pop("<", &span)?;
                match (a, b) {
                    (Value::Int(a), Value::Int(b)) => self.stack.push(Value::Bool(a < b)),
                    (a, b) => {
                        return Err(EvalError {
                            kind: EvalErrorKind::TypeMismatch {
                                op: "<".into(),
                                expected: "int, int".into(),
                                found: format!("{}, {}", a.type_name(), b.type_name()),
                            },
                            span: Some(span),
                        });
                    }
                }
            }

            Expr::Gt => {
                let b = self.pop(">", &span)?;
                let a = self.pop(">", &span)?;
                match (a, b) {
                    (Value::Int(a), Value::Int(b)) => self.stack.push(Value::Bool(a > b)),
                    (a, b) => {
                        return Err(EvalError {
                            kind: EvalErrorKind::TypeMismatch {
                                op: ">".into(),
                                expected: "int, int".into(),
                                found: format!("{}, {}", a.type_name(), b.type_name()),
                            },
                            span: Some(span),
                        });
                    }
                }
            }

            Expr::LtEq => {
                let b = self.pop("<=", &span)?;
                let a = self.pop("<=", &span)?;
                match (a, b) {
                    (Value::Int(a), Value::Int(b)) => self.stack.push(Value::Bool(a <= b)),
                    (a, b) => {
                        return Err(EvalError {
                            kind: EvalErrorKind::TypeMismatch {
                                op: "<=".into(),
                                expected: "int, int".into(),
                                found: format!("{}, {}", a.type_name(), b.type_name()),
                            },
                            span: Some(span),
                        });
                    }
                }
            }

            Expr::GtEq => {
                let b = self.pop(">=", &span)?;
                let a = self.pop(">=", &span)?;
                match (a, b) {
                    (Value::Int(a), Value::Int(b)) => self.stack.push(Value::Bool(a >= b)),
                    (a, b) => {
                        return Err(EvalError {
                            kind: EvalErrorKind::TypeMismatch {
                                op: ">=".into(),
                                expected: "int, int".into(),
                                found: format!("{}, {}", a.type_name(), b.type_name()),
                            },
                            span: Some(span),
                        });
                    }
                }
            }

            Expr::Swap => {
                let b = self.pop("~", &span)?;
                let a = self.pop("~", &span)?;
                self.stack.push(b);
                self.stack.push(a);
            }

            Expr::Dup => {
                // `a :` → `a a` — duplicate top of stack.
                let a = self.pop(":", &span)?;
                self.stack.push(a.clone());
                self.stack.push(a);
            }

            Expr::Over => {
                // `a b _` → `a b a` — copy second element to top.
                if self.stack.len() < 2 {
                    return Err(EvalError {
                        kind: EvalErrorKind::StackUnderflow {
                            op: "_".into(),
                            expected: 2,
                            found: self.stack.len(),
                        },
                        span: Some(span),
                    });
                }
                let second = self.stack[self.stack.len() - 2].clone();
                self.stack.push(second);
            }

            Expr::Block(body) => {
                debug!("push block ({} exprs)", body.len());
                self.stack.push(Value::Block(Block {
                    body: body.iter().map(|(e, s)| (e.clone(), s.clone())).collect(),
                }));
            }

            Expr::List(body) => {
                // Evaluate on a sub-stack, collect results.
                debug!("evaluating list ({} exprs)", body.len());
                let mut sub = Machine::new();
                sub.eval(body)?;
                self.stack.push(Value::List(sub.into_stack()));
            }

            Expr::Map => {
                // `[...] (block) &!` — apply block to each element.
                let block = self.pop("&!", &span)?;
                let list = self.pop("&!", &span)?;
                match (list, &block) {
                    (Value::List(items), Value::Block(b)) => {
                        let mut result = Vec::with_capacity(items.len());
                        for item in items {
                            let mut sub = Machine::new();
                            sub.stack.push(item);
                            sub.eval(&b.body)?;
                            // Take the top of the sub-stack as the mapped value.
                            match sub.stack.pop() {
                                Some(v) => result.push(v),
                                None => {
                                    return Err(EvalError {
                                        kind: EvalErrorKind::StackUnderflow {
                                            op: "&! (map body produced no value)".into(),
                                            expected: 1,
                                            found: 0,
                                        },
                                        span: Some(span),
                                    });
                                }
                            }
                        }
                        self.stack.push(Value::List(result));
                    }
                    (list, _) => {
                        return Err(EvalError {
                            kind: EvalErrorKind::TypeMismatch {
                                op: "&!".into(),
                                expected: "list, block".into(),
                                found: format!("{}, {}", list.type_name(), block.type_name()),
                            },
                            span: Some(span),
                        });
                    }
                }
            }

            Expr::Filter => {
                // `[...] (block) &?` — keep elements where block produces truthy.
                let block = self.pop("&?", &span)?;
                let list = self.pop("&?", &span)?;
                match (list, &block) {
                    (Value::List(items), Value::Block(b)) => {
                        let mut result = Vec::new();
                        for item in items {
                            let mut sub = Machine::new();
                            sub.stack.push(item.clone());
                            sub.eval(&b.body)?;
                            match sub.stack.pop() {
                                Some(v) if v.is_truthy() => result.push(item),
                                _ => {}
                            }
                        }
                        self.stack.push(Value::List(result));
                    }
                    (list, _) => {
                        return Err(EvalError {
                            kind: EvalErrorKind::TypeMismatch {
                                op: "&?".into(),
                                expected: "list, block".into(),
                                found: format!("{}, {}", list.type_name(), block.type_name()),
                            },
                            span: Some(span),
                        });
                    }
                }
            }

            Expr::Loop => {
                // `cond_block body_block &`
                // Pop body then condition. While cond leaves a truthy value on the stack,
                // execute body. The condition runs inline — it is ordinary stack code and
                // must leave exactly one value (consumed as the predicate). Use `:` to
                // inspect a value without consuming it: `(: 0 !=)` peeks at the top.
                let body = self.pop("&", &span)?;
                let cond = self.pop("&", &span)?;
                match (cond, body) {
                    (Value::Block(cond_block), Value::Block(body_block)) => loop {
                        self.eval(&cond_block.body)?;
                        let flag = self.pop("& (condition result)", &span)?;
                        if !flag.is_truthy() {
                            break;
                        }
                        self.eval(&body_block.body)?;
                    },
                    (c, b) => {
                        return Err(EvalError {
                            kind: EvalErrorKind::TypeMismatch {
                                op: "&".into(),
                                expected: "block, block".into(),
                                found: format!("{}, {}", c.type_name(), b.type_name()),
                            },
                            span: Some(span),
                        });
                    }
                }
            }

            Expr::Dollar | Expr::At => {
                // Reserved — not yet implemented.
                debug!(?expr, "reserved operator (no-op)");
            }
        }

        trace!(stack_depth = self.stack.len(), "after eval");
        Ok(())
    }

    fn apply_op(
        &self,
        op: Op,
        a: Value,
        b: Value,
        span: &std::ops::Range<usize>,
    ) -> Result<Value, EvalError> {
        match (&a, &b) {
            (Value::Int(a), Value::Int(b)) => {
                let result = match op {
                    Op::Add => a + b,
                    Op::Sub => a - b,
                    Op::Mul => a * b,
                    Op::Div => {
                        if b.is_zero() {
                            return Err(EvalError {
                                kind: EvalErrorKind::DivisionByZero,
                                span: Some(span.clone()),
                            });
                        }
                        a / b
                    }
                    Op::Mod => {
                        if b.is_zero() {
                            return Err(EvalError {
                                kind: EvalErrorKind::DivisionByZero,
                                span: Some(span.clone()),
                            });
                        }
                        a % b
                    }
                    Op::Pow => {
                        let exp = b.to_u32().ok_or_else(|| EvalError {
                            kind: EvalErrorKind::TypeMismatch {
                                op: "^".into(),
                                expected: "non-negative exponent that fits in u32".into(),
                                found: format!("{b}"),
                            },
                            span: Some(span.clone()),
                        })?;
                        num_traits::pow::Pow::pow(a, exp)
                    }
                };
                Ok(Value::Int(result))
            }
            _ => Err(EvalError {
                kind: EvalErrorKind::TypeMismatch {
                    op: op.to_string(),
                    expected: "int, int".into(),
                    found: format!("{}, {}", a.type_name(), b.type_name()),
                },
                span: Some(span.clone()),
            }),
        }
    }

    fn pop(&mut self, op: &str, span: &std::ops::Range<usize>) -> Result<Value, EvalError> {
        self.stack.pop().ok_or_else(|| EvalError {
            kind: EvalErrorKind::StackUnderflow {
                op: op.into(),
                expected: 1,
                found: 0,
            },
            span: Some(span.clone()),
        })
    }
}

impl Default for Machine {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{lexer, parser};

    fn run(src: &str) -> Vec<Value> {
        let tokens = lexer::lex(src).expect("lex failed");
        let ast = parser::parse(&tokens, src.len()).expect("parse failed");
        let mut machine = Machine::new();
        machine.eval(&ast).expect("eval failed");
        machine.into_stack()
    }

    fn run_err(src: &str) -> EvalError {
        let tokens = lexer::lex(src).expect("lex failed");
        let ast = parser::parse(&tokens, src.len()).expect("parse failed");
        let mut machine = Machine::new();
        machine.eval(&ast).expect_err("expected eval error")
    }

    #[test]
    fn push_integer() {
        assert_eq!(run("42"), vec![Value::Int(42.into())]);
    }

    #[test]
    fn addition() {
        assert_eq!(run("3 4 +"), vec![Value::Int(7.into())]);
    }

    #[test]
    fn subtraction() {
        assert_eq!(run("10 3 -"), vec![Value::Int(7.into())]);
    }

    #[test]
    fn multiplication() {
        assert_eq!(run("6 7 *"), vec![Value::Int(42.into())]);
    }

    #[test]
    fn division() {
        assert_eq!(run("15 4 /"), vec![Value::Int(3.into())]);
    }

    #[test]
    fn modulo() {
        assert_eq!(run("15 4 %"), vec![Value::Int(3.into())]);
    }

    #[test]
    fn power() {
        assert_eq!(run("2 10 ^"), vec![Value::Int(1024.into())]);
    }

    #[test]
    fn chained_arithmetic() {
        // (3 + 4) * 2 = 14
        assert_eq!(run("3 4 + 2 *"), vec![Value::Int(14.into())]);
    }

    #[test]
    fn swap() {
        assert_eq!(
            run("1 2 ~"),
            vec![Value::Int(2.into()), Value::Int(1.into())]
        );
    }

    #[test]
    fn dup() {
        assert_eq!(run("5 :"), vec![Value::Int(5.into()), Value::Int(5.into())]);
    }

    #[test]
    fn dup_square() {
        // n : * => n^2
        assert_eq!(run("7 : *"), vec![Value::Int(49.into())]);
    }

    #[test]
    fn over() {
        // `1 2 _` → [1, 2, 1]
        assert_eq!(
            run("1 2 _"),
            vec![
                Value::Int(1.into()),
                Value::Int(2.into()),
                Value::Int(1.into())
            ]
        );
    }

    #[test]
    fn block_execute() {
        // Push 3, push block (4 +), execute → 7.
        assert_eq!(run("3 (4 +) !"), vec![Value::Int(7.into())]);
    }

    #[test]
    fn list_creation() {
        assert_eq!(
            run("[1 2 3]"),
            vec![Value::List(vec![
                Value::Int(1.into()),
                Value::Int(2.into()),
                Value::Int(3.into()),
            ])]
        );
    }

    #[test]
    fn list_with_arithmetic() {
        // [3 4 +] should evaluate to [7].
        assert_eq!(
            run("[3 4 +]"),
            vec![Value::List(vec![Value::Int(7.into())])]
        );
    }

    #[test]
    fn compose_lists() {
        assert_eq!(
            run("[1 2] [3 4] |"),
            vec![Value::List(vec![
                Value::Int(1.into()),
                Value::Int(2.into()),
                Value::Int(3.into()),
                Value::Int(4.into()),
            ])]
        );
    }

    #[test]
    fn equality() {
        assert_eq!(run("3 3 =="), vec![Value::Bool(true)]);
        assert_eq!(run("3 4 =="), vec![Value::Bool(false)]);
    }

    #[test]
    fn conditional_truthy() {
        // `5 1 ?` — 1 is truthy, so 5 stays.
        assert_eq!(run("5 1 ?"), vec![Value::Int(5.into())]);
    }

    #[test]
    fn conditional_falsy() {
        // `5 0 ?` — 0 is falsy, so 5 is also popped.
        assert_eq!(run("5 0 ?"), vec![]);
    }

    #[test]
    fn ternary_truthy() {
        // `10 20 1 ??` — 1 is truthy, keep 20 (discard 10).
        assert_eq!(run("10 20 1 ??"), vec![Value::Int(20.into())]);
    }

    #[test]
    fn ternary_falsy() {
        // `10 20 0 ??` — 0 is falsy, keep 10 (discard 20).
        assert_eq!(run("10 20 0 ??"), vec![Value::Int(10.into())]);
    }

    #[test]
    fn map() {
        // [1 2 3] (1 +) &! → [2 3 4]
        assert_eq!(
            run("[1 2 3] (1 +) &!"),
            vec![Value::List(vec![
                Value::Int(2.into()),
                Value::Int(3.into()),
                Value::Int(4.into()),
            ])]
        );
    }

    #[test]
    fn filter() {
        // [1 2 3 4 5] (3 >) -- wait, we don't have > yet.
        // Let's use equality: [1 2 3] (2 ==) &? → [2]
        // Actually `==` returns bool. Let's filter for values equal to 2.
        // Hmm, we need a proper predicate. Since we don't have comparison yet
        // beyond ==, let's test truthiness: filter non-zero from a list.
        // Actually, any int is truthy except 0, so:
        // [0 1 0 2 0 3] (_ *) &? → multiply by self (square), non-zero stays.
        // Simpler: [1 2 3] (1 ==) &? → [1]
        assert_eq!(
            run("[1 2 3] (1 ==) &?"),
            vec![Value::List(vec![Value::Int(1.into())])]
        );
    }

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
        // 2^100 is a big number.
        let result = run("2 100 ^");
        assert_eq!(result.len(), 1);
        let expected: BigInt = BigInt::from(2).pow(100);
        assert_eq!(result[0], Value::Int(expected));
    }

    #[test]
    fn multiple_values_on_stack() {
        assert_eq!(
            run("1 2 3"),
            vec![
                Value::Int(1.into()),
                Value::Int(2.into()),
                Value::Int(3.into())
            ]
        );
    }

    #[test]
    fn not_equal() {
        assert_eq!(run("3 4 !="), vec![Value::Bool(true)]);
        assert_eq!(run("3 3 !="), vec![Value::Bool(false)]);
    }

    #[test]
    fn less_than() {
        assert_eq!(run("3 4 <"), vec![Value::Bool(true)]);
        assert_eq!(run("4 3 <"), vec![Value::Bool(false)]);
        assert_eq!(run("3 3 <"), vec![Value::Bool(false)]);
    }

    #[test]
    fn greater_than() {
        assert_eq!(run("4 3 >"), vec![Value::Bool(true)]);
        assert_eq!(run("3 4 >"), vec![Value::Bool(false)]);
        assert_eq!(run("3 3 >"), vec![Value::Bool(false)]);
    }

    #[test]
    fn less_than_or_equal() {
        assert_eq!(run("3 4 <="), vec![Value::Bool(true)]);
        assert_eq!(run("3 3 <="), vec![Value::Bool(true)]);
        assert_eq!(run("4 3 <="), vec![Value::Bool(false)]);
    }

    #[test]
    fn greater_than_or_equal() {
        assert_eq!(run("4 3 >="), vec![Value::Bool(true)]);
        assert_eq!(run("3 3 >="), vec![Value::Bool(true)]);
        assert_eq!(run("3 4 >="), vec![Value::Bool(false)]);
    }

    #[test]
    fn loop_countdown() {
        // `: 0 !=` peeks at top without consuming it (dup, compare, pop bool)
        // 5 (: 0 !=) (1 -) & → while top != 0, decrement → result 0
        assert_eq!(run("5 (: 0 !=) (1 -) &"), vec![Value::Int(0.into())]);
    }

    #[test]
    fn loop_zero_iters() {
        // Condition is immediately false — body never runs.
        assert_eq!(run("0 (: 0 !=) (1 +) &"), vec![Value::Int(0.into())]);
    }

    #[test]
    fn filter_with_comparison() {
        // [1 2 3 4 5] (3 >) &? → keep values where x > 3 → [4 5]
        assert_eq!(
            run("[1 2 3 4 5] (3 >) &?"),
            vec![Value::List(vec![
                Value::Int(4.into()),
                Value::Int(5.into())
            ])]
        );
    }
}
