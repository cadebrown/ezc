use std::fmt;

use num_bigint::BigInt;

use crate::ast::{Expr, Spanned};

/// The fundamental value type in ezc.
///
/// Everything on the stack is a `Value`. The language is dynamically typed --
/// operators check value variants at runtime and produce type errors on mismatch.
#[derive(Debug, Clone)]
pub enum Value {
    /// Arbitrary-precision integer (the default numeric type).
    Int(BigInt),
    /// Boolean (produced by comparison operators).
    Bool(bool),
    /// A block of deferred code, captured from `(...)` syntax.
    Block(Block),
    /// An eagerly-evaluated list, produced by `[...]` syntax.
    List(Vec<Value>),
}

/// A block is a captured sequence of expressions for deferred execution.
///
/// Created by `(...)` syntax. Executed by the `!` operator, which pops the
/// block and evaluates its body against the current stack.
#[derive(Debug, Clone)]
pub struct Block {
    pub body: Vec<Spanned<Expr>>,
}

impl Value {
    /// Returns a human-readable type name for error messages.
    pub fn type_name(&self) -> &'static str {
        match self {
            Value::Int(_) => "int",
            Value::Bool(_) => "bool",
            Value::Block(_) => "block",
            Value::List(_) => "list",
        }
    }

    /// Returns `true` if this value is "truthy" for conditional operators.
    ///
    /// - `Int`: truthy if non-zero
    /// - `Bool`: truthy if `true`
    /// - `Block`: always truthy
    /// - `List`: truthy if non-empty
    pub fn is_truthy(&self) -> bool {
        match self {
            Value::Int(n) => *n != BigInt::ZERO,
            Value::Bool(b) => *b,
            Value::Block(_) => true,
            Value::List(items) => !items.is_empty(),
        }
    }
}

impl fmt::Display for Value {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Value::Int(n) => write!(f, "{n}"),
            Value::Bool(b) => write!(f, "{b}"),
            Value::Block(_) => write!(f, "(...)"),
            Value::List(items) => {
                write!(f, "[")?;
                for (i, item) in items.iter().enumerate() {
                    if i > 0 {
                        write!(f, " ")?;
                    }
                    write!(f, "{item}")?;
                }
                write!(f, "]")
            }
        }
    }
}

impl PartialEq for Value {
    fn eq(&self, other: &Self) -> bool {
        match (self, other) {
            (Value::Int(a), Value::Int(b)) => a == b,
            (Value::Bool(a), Value::Bool(b)) => a == b,
            (Value::List(a), Value::List(b)) => a == b,
            // Blocks are never equal (no structural comparison).
            _ => false,
        }
    }
}

impl Eq for Value {}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn display_int() {
        assert_eq!(Value::Int(42.into()).to_string(), "42");
        assert_eq!(Value::Int((-7).into()).to_string(), "-7");
    }

    #[test]
    fn display_bool() {
        assert_eq!(Value::Bool(true).to_string(), "true");
        assert_eq!(Value::Bool(false).to_string(), "false");
    }

    #[test]
    fn display_list() {
        let list = Value::List(vec![
            Value::Int(1.into()),
            Value::Int(2.into()),
            Value::Int(3.into()),
        ]);
        assert_eq!(list.to_string(), "[1 2 3]");
    }

    #[test]
    fn display_empty_list() {
        assert_eq!(Value::List(vec![]).to_string(), "[]");
    }

    #[test]
    fn display_block() {
        let block = Value::Block(Block { body: vec![] });
        assert_eq!(block.to_string(), "(...)");
    }

    #[test]
    fn truthiness() {
        assert!(Value::Int(1.into()).is_truthy());
        assert!(!Value::Int(0.into()).is_truthy());
        assert!(Value::Bool(true).is_truthy());
        assert!(!Value::Bool(false).is_truthy());
        assert!(Value::Block(Block { body: vec![] }).is_truthy());
        assert!(Value::List(vec![Value::Int(1.into())]).is_truthy());
        assert!(!Value::List(vec![]).is_truthy());
    }

    #[test]
    fn equality() {
        assert_eq!(Value::Int(42.into()), Value::Int(42.into()));
        assert_ne!(Value::Int(1.into()), Value::Int(2.into()));
        assert_eq!(Value::Bool(true), Value::Bool(true));
        assert_ne!(Value::Int(1.into()), Value::Bool(true));
        // Blocks are never equal.
        let b1 = Value::Block(Block { body: vec![] });
        let b2 = Value::Block(Block { body: vec![] });
        assert_ne!(b1, b2);
    }
}
