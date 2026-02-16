use crate::{
    error::{ErrorCode, EzcError},
    ezcbc::{BlockValue, Bytecode, OpCode, Value},
    Span,
};

#[derive(Debug, Clone, PartialEq, Eq, Default)]
pub struct ExecutionResult {
    pub stack: Vec<Value>,
    pub stdout: String,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ExecutionStep {
    pub depth: usize,
    pub ip: usize,
    pub op: String,
    pub span: Span,
    pub stack_before: Vec<Value>,
    pub stack_after: Vec<Value>,
}

#[derive(Debug, Clone, PartialEq, Eq, Default)]
pub struct ExecutionTrace {
    pub result: ExecutionResult,
    pub steps: Vec<ExecutionStep>,
}

#[derive(Debug, Default, Clone)]
pub struct Vm {
    stack: Vec<Value>,
    output: Vec<String>,
}

impl Vm {
    #[tracing::instrument(skip(self, bytecode), fields(instruction_count = bytecode.instructions.len()))]
    pub fn execute(&mut self, bytecode: &Bytecode) -> Result<ExecutionResult, EzcError> {
        self.execute_internal(bytecode, false)
            .map(|trace| trace.result)
    }

    pub fn execute_verbose(&mut self, bytecode: &Bytecode) -> Result<ExecutionTrace, EzcError> {
        self.execute_internal(bytecode, true)
    }

    fn execute_internal(
        &mut self,
        bytecode: &Bytecode,
        capture_steps: bool,
    ) -> Result<ExecutionTrace, EzcError> {
        let output_start = self.output.len();
        let mut steps = Vec::new();
        self.execute_bytecode(bytecode, 0, capture_steps, &mut steps)?;

        Ok(ExecutionTrace {
            result: ExecutionResult {
                // Keep VM output persistent internally, but only report newly produced lines.
                stack: self.stack.clone(),
                stdout: self.output[output_start..].join("\n"),
            },
            steps,
        })
    }

    fn execute_bytecode(
        &mut self,
        bytecode: &Bytecode,
        depth: usize,
        capture_steps: bool,
        steps: &mut Vec<ExecutionStep>,
    ) -> Result<(), EzcError> {
        for (ip, instruction) in bytecode.instructions.iter().enumerate() {
            tracing::trace!(
                depth,
                ip,
                op = instruction.op.mnemonic(),
                stack = ?self.stack,
                "executing instruction"
            );

            let stack_before = if capture_steps {
                Some(self.stack.clone())
            } else {
                None
            };

            self.execute_instruction(
                &instruction.op,
                &instruction.span,
                depth,
                capture_steps,
                steps,
            )?;

            if let Some(stack_before) = stack_before {
                steps.push(ExecutionStep {
                    depth,
                    ip,
                    op: instruction.op.mnemonic().to_string(),
                    span: instruction.span.clone(),
                    stack_before,
                    stack_after: self.stack.clone(),
                });
            }
        }

        Ok(())
    }

    fn execute_instruction(
        &mut self,
        op: &OpCode,
        span: &Span,
        depth: usize,
        capture_steps: bool,
        steps: &mut Vec<ExecutionStep>,
    ) -> Result<(), EzcError> {
        match op {
            OpCode::Push(value) => self.stack.push(value.clone()),
            OpCode::Add => {
                let (lhs, rhs) = self.pop2_int("+", span)?;
                let value = lhs.checked_add(rhs).ok_or_else(|| {
                    EzcError::new(
                        ErrorCode::RuntimeOverflow,
                        format!("integer overflow in `{lhs} + {rhs}`"),
                        span.clone(),
                    )
                })?;
                self.stack.push(Value::Int(value));
            }
            OpCode::Sub => {
                let (lhs, rhs) = self.pop2_int("-", span)?;
                let value = lhs.checked_sub(rhs).ok_or_else(|| {
                    EzcError::new(
                        ErrorCode::RuntimeOverflow,
                        format!("integer overflow in `{lhs} - {rhs}`"),
                        span.clone(),
                    )
                })?;
                self.stack.push(Value::Int(value));
            }
            OpCode::Mul => {
                let (lhs, rhs) = self.pop2_int("*", span)?;
                let value = lhs.checked_mul(rhs).ok_or_else(|| {
                    EzcError::new(
                        ErrorCode::RuntimeOverflow,
                        format!("integer overflow in `{lhs} * {rhs}`"),
                        span.clone(),
                    )
                })?;
                self.stack.push(Value::Int(value));
            }
            OpCode::Div => {
                let (lhs, rhs) = self.pop2_int("/", span)?;
                if rhs == 0 {
                    return Err(EzcError::new(
                        ErrorCode::RuntimeDivisionByZero,
                        "division by zero",
                        span.clone(),
                    ));
                }
                self.stack.push(Value::Int(lhs / rhs));
            }
            OpCode::Mod => {
                let (lhs, rhs) = self.pop2_int("%", span)?;
                if rhs == 0 {
                    return Err(EzcError::new(
                        ErrorCode::RuntimeModuloByZero,
                        "modulo by zero",
                        span.clone(),
                    ));
                }
                self.stack.push(Value::Int(lhs % rhs));
            }
            OpCode::Dup => {
                let value = self.peek("dup", span)?;
                self.stack.push(value);
            }
            OpCode::Drop => {
                self.pop1("del", span)?;
            }
            OpCode::Swap => {
                let rhs = self.pop1("swp", span)?;
                let lhs = self.pop1("swp", span)?;
                self.stack.push(rhs);
                self.stack.push(lhs);
            }
            OpCode::Over => {
                if self.stack.len() < 2 {
                    return Err(EzcError::new(
                        ErrorCode::RuntimeStackUnderflow,
                        "stack underflow in `ovr`",
                        span.clone(),
                    ));
                }
                let copied = self.stack[self.stack.len() - 2].clone();
                self.stack.push(copied);
            }
            OpCode::Eq => {
                let rhs = self.pop1("=", span)?;
                let lhs = self.pop1("=", span)?;
                self.stack.push(Value::Int((lhs == rhs) as i64));
            }
            OpCode::Lt => {
                let (lhs, rhs) = self.pop2_int("<", span)?;
                self.stack.push(Value::Int((lhs < rhs) as i64));
            }
            OpCode::Gt => {
                let (lhs, rhs) = self.pop2_int(">", span)?;
                self.stack.push(Value::Int((lhs > rhs) as i64));
            }
            OpCode::And => {
                let rhs = self.pop1("&", span)?;
                let lhs = self.pop1("&", span)?;
                self.stack
                    .push(Value::Int((lhs.is_truthy() && rhs.is_truthy()) as i64));
            }
            OpCode::Or => {
                let rhs = self.pop1("|", span)?;
                let lhs = self.pop1("|", span)?;
                self.stack
                    .push(Value::Int((lhs.is_truthy() || rhs.is_truthy()) as i64));
            }
            OpCode::Not => {
                let value = self.pop1("not", span)?;
                self.stack.push(Value::Int((!value.is_truthy()) as i64));
            }
            OpCode::Print => {
                let value = self.pop1("prt", span)?;
                match value {
                    Value::Text(text) => self.output.push(text),
                    other => self.output.push(other.to_string()),
                }
            }
            OpCode::Exec => {
                let block = self.pop_block("!", span)?;
                self.execute_bytecode(&block.bytecode, depth + 1, capture_steps, steps)?;
            }
            OpCode::Select => {
                let condition = self.pop1("?", span)?;
                let if_false = self.pop1("?", span)?;
                let if_true = self.pop1("?", span)?;
                if condition.is_truthy() {
                    self.stack.push(if_true);
                } else {
                    self.stack.push(if_false);
                }
            }
            OpCode::Loop => {
                let block = self.pop_block("^", span)?;
                loop {
                    // Loop blocks are responsible for leaving one continuation value.
                    self.execute_bytecode(&block.bytecode, depth + 1, capture_steps, steps)?;
                    let condition = self.pop1("^", span)?;
                    if !condition.is_truthy() {
                        break;
                    }
                }
            }
        }

        Ok(())
    }

    fn pop1(&mut self, op: &str, span: &Span) -> Result<Value, EzcError> {
        self.stack.pop().ok_or_else(|| {
            EzcError::new(
                ErrorCode::RuntimeStackUnderflow,
                format!("stack underflow in `{op}`"),
                span.clone(),
            )
        })
    }

    fn peek(&self, op: &str, span: &Span) -> Result<Value, EzcError> {
        self.stack.last().cloned().ok_or_else(|| {
            EzcError::new(
                ErrorCode::RuntimeStackUnderflow,
                format!("stack underflow in `{op}`"),
                span.clone(),
            )
        })
    }

    fn pop2_int(&mut self, op: &str, span: &Span) -> Result<(i64, i64), EzcError> {
        let rhs = self.pop_int(op, span)?;
        let lhs = self.pop_int(op, span)?;
        Ok((lhs, rhs))
    }

    fn pop_int(&mut self, op: &str, span: &Span) -> Result<i64, EzcError> {
        let value = self.pop1(op, span)?;
        match value {
            Value::Int(number) => Ok(number),
            other => Err(EzcError::new(
                ErrorCode::RuntimeTypeMismatch,
                format!("type error in `{op}`: expected integer, got `{other}`"),
                span.clone(),
            )),
        }
    }

    fn pop_block(&mut self, op: &str, span: &Span) -> Result<BlockValue, EzcError> {
        let value = self.pop1(op, span)?;
        match value {
            Value::Block(block) => Ok(block),
            other => Err(EzcError::new(
                ErrorCode::RuntimeTypeMismatch,
                format!("type error in `{op}`: expected block, got `{other}`"),
                span.clone(),
            )),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{compile_source, error::ErrorCode, ezcbc::Value};

    use super::*;

    #[test]
    fn executes_arithmetic_and_stack_ops() {
        let bytecode = compile_source("test.ezc", "3 4 + dup *").expect("compile should succeed");

        let mut vm = Vm::default();
        let result = vm.execute(&bytecode).expect("execution should succeed");

        assert_eq!(result.stack, vec![Value::Int(49)]);
        assert!(result.stdout.is_empty());
    }

    #[test]
    fn executes_delayed_block_with_bang() {
        let bytecode =
            compile_source("test.ezc", "(5 dup * prt)!").expect("compile should succeed");

        let mut vm = Vm::default();
        let result = vm.execute(&bytecode).expect("execution should succeed");

        assert_eq!(result.stdout, "25");
        assert_eq!(result.stack, vec![]);
    }

    #[test]
    fn prints_text_without_quotes() {
        let bytecode = compile_source("test.ezc", "\"hello\" prt").expect("compile should succeed");

        let mut vm = Vm::default();
        let result = vm.execute(&bytecode).expect("execution should succeed");

        assert_eq!(result.stdout, "hello");
    }

    #[test]
    fn executes_conditional_select_operator() {
        let bytecode = compile_source("test.ezc", "111 222 0 ?").expect("compile should succeed");

        let mut vm = Vm::default();
        let result = vm.execute(&bytecode).expect("execution should succeed");

        assert_eq!(result.stack, vec![Value::Int(222)]);
    }

    #[test]
    fn executes_loop_operator() {
        let bytecode =
            compile_source("test.ezc", "3 (dup prt 1 - dup) ^").expect("compile should succeed");

        let mut vm = Vm::default();
        let result = vm.execute(&bytecode).expect("execution should succeed");

        assert_eq!(result.stdout, "3\n2\n1");
        assert_eq!(result.stack, vec![Value::Int(0)]);
    }

    #[test]
    fn reports_underflow() {
        let bytecode = compile_source("test.ezc", "+").expect("compile should succeed");

        let mut vm = Vm::default();
        let err = vm.execute(&bytecode).expect_err("execution should fail");

        assert_eq!(err.code, ErrorCode::RuntimeStackUnderflow);
        assert!(err.message.contains("underflow"));
    }

    #[test]
    fn captures_verbose_steps() {
        let bytecode = compile_source("test.ezc", "2 3 +").expect("compile should succeed");

        let mut vm = Vm::default();
        let trace = vm
            .execute_verbose(&bytecode)
            .expect("execution should succeed");

        assert_eq!(trace.steps.len(), 3);
        assert_eq!(trace.result.stack, vec![Value::Int(5)]);
    }

    #[test]
    fn comma_is_dup_alias_not_print() {
        let bytecode = compile_source("test.ezc", "7 , +").expect("compile should succeed");

        let mut vm = Vm::default();
        let result = vm.execute(&bytecode).expect("execution should succeed");

        assert_eq!(result.stack, vec![Value::Int(14)]);
        assert!(result.stdout.is_empty());
    }

    #[test]
    fn dot_is_drop_alias() {
        let bytecode = compile_source("test.ezc", "9 4 .").expect("compile should succeed");

        let mut vm = Vm::default();
        let result = vm.execute(&bytecode).expect("execution should succeed");

        assert_eq!(result.stack, vec![Value::Int(9)]);
        assert!(result.stdout.is_empty());
    }

    #[test]
    fn symbolic_logical_ops_work() {
        let bytecode = compile_source("test.ezc", "1 0 | 1 &").expect("compile should succeed");

        let mut vm = Vm::default();
        let result = vm.execute(&bytecode).expect("execution should succeed");

        assert_eq!(result.stack, vec![Value::Int(1)]);
        assert!(result.stdout.is_empty());
    }
}
