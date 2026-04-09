use std::fmt;
use std::sync::Arc;

use num_bigint::BigInt;

use crate::ast::{Expr, Spanned};
use crate::number::Number;

/// The fundamental value type in ezc.
///
/// Everything on the stack is a `Value`. The language is dynamically typed —
/// operators check value variants at runtime and produce type errors on mismatch.
///
/// There is no separate boolean type. Comparisons push `1` (truthy) or `0` (falsy).
#[derive(Debug, Clone)]
pub enum Value {
    /// Any numeric value (arbitrary-precision int, fixed-width int, or float).
    Num(Number),
    /// Immutable string.
    Str(EzStr),
    /// Immutable binary blob.
    Bin(EzBin),
    /// A block of deferred code, captured from `(...)` syntax.
    Block(Block),
    /// An eagerly-evaluated list, produced by `[...]` syntax.
    List(Vec<Value>),
}

/// Immutable interned string. Pointer-equal when interned by the same engine.
#[derive(Debug, Clone, Eq)]
pub struct EzStr(pub Arc<str>);

/// Immutable interned binary blob. Pointer-equal when interned by the same engine.
#[derive(Debug, Clone, Eq)]
pub struct EzBin(pub Arc<[u8]>);

impl PartialEq for EzStr {
    fn eq(&self, other: &Self) -> bool {
        Arc::ptr_eq(&self.0, &other.0) || *self.0 == *other.0
    }
}

impl std::hash::Hash for EzStr {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.0.hash(state);
    }
}

impl PartialEq for EzBin {
    fn eq(&self, other: &Self) -> bool {
        Arc::ptr_eq(&self.0, &other.0) || *self.0 == *other.0
    }
}

impl std::hash::Hash for EzBin {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.0.hash(state);
    }
}

/// A block is a sequence of expressions for deferred execution.
///
/// Created by `(...)` syntax. Executed by the `!` operator, which pops the
/// block and evaluates its body against the current stack. Blocks use dynamic
/// scoping — they see whatever bindings exist at execution time.
#[derive(Debug, Clone)]
pub struct Block {
    pub body: Vec<Spanned<Expr>>,
}

// ── Convenience constructors ──────────────────────────────────────────────

impl Value {
    /// Shorthand for a BigInt value.
    pub fn int(n: impl Into<BigInt>) -> Self {
        Value::Num(Number::Int(Arc::new(n.into())))
    }

    /// Format for printing (`:` and `wl`). Strings print raw content
    /// without quotes. Everything else uses `Display`.
    pub fn print_string(&self) -> String {
        match self {
            Value::Str(s) => s.0.to_string(),
            other => other.to_string(),
        }
    }

    /// Returns a human-readable type name for error messages.
    pub fn type_name(&self) -> &'static str {
        match self {
            Value::Num(n) => n.type_name(),
            Value::Str(_) => "str",
            Value::Bin(_) => "bin",
            Value::Block(_) => "block",
            Value::List(_) => "list",
        }
    }

    /// Returns `true` if this value is "truthy" for conditional operators.
    ///
    /// - Num: truthy if non-zero
    /// - Str: truthy if non-empty
    /// - Bin: truthy if non-empty
    /// - Block: always truthy
    /// - List: truthy if non-empty
    pub fn is_truthy(&self) -> bool {
        match self {
            Value::Num(n) => !n.is_zero(),
            Value::Str(s) => !s.0.is_empty(),
            Value::Bin(b) => !b.0.is_empty(),
            Value::Block(_) => true,
            Value::List(items) => !items.is_empty(),
        }
    }
}

impl EzStr {
    pub fn new(s: &str) -> Self {
        EzStr(Arc::from(s))
    }
}

impl EzBin {
    pub fn new(data: &[u8]) -> Self {
        EzBin(Arc::from(data))
    }
}

// ── Display ───────────────────────────────────────────────────────────────

impl fmt::Display for Value {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Value::Num(n) => write!(f, "{n}"),
            Value::Str(s) => write!(f, "{s}"),
            Value::Bin(b) => write!(f, "{b}"),
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

impl fmt::Display for EzStr {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "\"{}\"", self.0)
    }
}

impl fmt::Display for EzBin {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "0x")?;
        for byte in self.0.iter() {
            write!(f, "{byte:02x}")?;
        }
        Ok(())
    }
}

// ── PartialEq ─────────────────────────────────────────────────────────────

impl PartialEq for Value {
    fn eq(&self, other: &Self) -> bool {
        match (self, other) {
            (Value::Num(a), Value::Num(b)) => a == b,
            (Value::Str(a), Value::Str(b)) => a == b,
            (Value::Bin(a), Value::Bin(b)) => a == b,
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
        assert_eq!(Value::int(42).to_string(), "42");
        assert_eq!(Value::int(-7).to_string(), "-7");
    }

    #[test]
    #[allow(clippy::approx_constant)]
    fn display_typed_nums() {
        assert_eq!(Value::Num(Number::U8(255)).to_string(), "255u8");
        assert_eq!(Value::Num(Number::I32(-42)).to_string(), "-42i32");
        assert_eq!(Value::Num(Number::F64(3.14)).to_string(), "3.14");
    }

    #[test]
    fn display_str() {
        assert_eq!(Value::Str(EzStr::new("hello")).to_string(), "\"hello\"");
    }

    #[test]
    fn display_bin() {
        assert_eq!(Value::Bin(EzBin::new(&[0xDE, 0xAD])).to_string(), "0xdead");
    }

    #[test]
    fn display_list() {
        let list = Value::List(vec![Value::int(1), Value::int(2), Value::int(3)]);
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
        assert!(Value::int(1).is_truthy());
        assert!(!Value::int(0).is_truthy());
        assert!(Value::Str(EzStr::new("hi")).is_truthy());
        assert!(!Value::Str(EzStr::new("")).is_truthy());
        assert!(Value::Block(Block { body: vec![] }).is_truthy());
        assert!(Value::List(vec![Value::int(1)]).is_truthy());
        assert!(!Value::List(vec![]).is_truthy());
    }

    #[test]
    fn equality() {
        assert_eq!(Value::int(42), Value::int(42));
        assert_ne!(Value::int(1), Value::int(2));
        assert_eq!(Value::Str(EzStr::new("a")), Value::Str(EzStr::new("a")));
        // Blocks are never equal.
        let b1 = Value::Block(Block { body: vec![] });
        let b2 = Value::Block(Block { body: vec![] });
        assert_ne!(b1, b2);
    }
}
