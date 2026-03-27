//! Fixed-width numeric types and arithmetic.
//!
//! The `Number` enum represents all numeric values in ezc: arbitrary-precision
//! integers (`Int`), fixed-width unsigned/signed integers, and floating-point.
//! Arithmetic within a family (e.g. u8 + u32) promotes to the wider type.
//! Cross-family arithmetic (e.g. u8 + i8, or Int + u8) is a type error.

use std::fmt;
use std::sync::Arc;

use ethnum::{I256, U256};
use half::f16;
use num_bigint::BigInt;
use num_traits::{ToPrimitive, Zero};

/// Numeric family — determines which types can interoperate.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum NumFamily {
    Int,      // arbitrary precision BigInt
    Unsigned, // u8..u256
    Signed,   // i8..i256
    Float,    // f16..f64
}

/// Bit width for fixed-width types.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum BitWidth {
    W8,
    W16,
    W32,
    W64,
    W128,
    W256,
}

/// A numeric value — the workhorse type for all ezc numbers.
#[derive(Debug, Clone)]
pub enum Number {
    // Arbitrary precision (Arc for interning — cheap clone + pointer equality)
    Int(Arc<BigInt>),
    // Unsigned fixed-width
    U8(u8),
    U16(u16),
    U32(u32),
    U64(u64),
    U128(u128),
    U256(U256),
    // Signed fixed-width
    I8(i8),
    I16(i16),
    I32(i32),
    I64(i64),
    I128(i128),
    I256(I256),
    // Floating point
    F16(f16),
    F32(f32),
    F64(f64),
}

impl Number {
    pub fn family(&self) -> NumFamily {
        match self {
            Number::Int(_) => NumFamily::Int,
            Number::U8(_)
            | Number::U16(_)
            | Number::U32(_)
            | Number::U64(_)
            | Number::U128(_)
            | Number::U256(_) => NumFamily::Unsigned,
            Number::I8(_)
            | Number::I16(_)
            | Number::I32(_)
            | Number::I64(_)
            | Number::I128(_)
            | Number::I256(_) => NumFamily::Signed,
            Number::F16(_) | Number::F32(_) | Number::F64(_) => NumFamily::Float,
        }
    }

    pub fn type_name(&self) -> &'static str {
        match self {
            Number::Int(_) => "int",
            Number::U8(_) => "u8",
            Number::U16(_) => "u16",
            Number::U32(_) => "u32",
            Number::U64(_) => "u64",
            Number::U128(_) => "u128",
            Number::U256(_) => "u256",
            Number::I8(_) => "i8",
            Number::I16(_) => "i16",
            Number::I32(_) => "i32",
            Number::I64(_) => "i64",
            Number::I128(_) => "i128",
            Number::I256(_) => "i256",
            Number::F16(_) => "f16",
            Number::F32(_) => "f32",
            Number::F64(_) => "f64",
        }
    }

    pub fn is_zero(&self) -> bool {
        match self {
            Number::Int(n) => **n == BigInt::ZERO,
            Number::U8(n) => *n == 0,
            Number::U16(n) => *n == 0,
            Number::U32(n) => *n == 0,
            Number::U64(n) => *n == 0,
            Number::U128(n) => *n == 0,
            Number::U256(n) => *n == U256::ZERO,
            Number::I8(n) => *n == 0,
            Number::I16(n) => *n == 0,
            Number::I32(n) => *n == 0,
            Number::I64(n) => *n == 0,
            Number::I128(n) => *n == 0,
            Number::I256(n) => *n == I256::ZERO,
            Number::F16(n) => *n == f16::ZERO,
            Number::F32(n) => *n == 0.0,
            Number::F64(n) => *n == 0.0,
        }
    }

    /// Convert any numeric value to BigInt (lossy for floats).
    pub fn to_bigint_lossy(&self) -> BigInt {
        match self {
            Number::Int(n) => n.as_ref().clone(),
            Number::U8(n) => BigInt::from(*n),
            Number::U16(n) => BigInt::from(*n),
            Number::U32(n) => BigInt::from(*n),
            Number::U64(n) => BigInt::from(*n),
            Number::U128(n) => BigInt::from(*n),
            Number::U256(n) => {
                let bytes = n.to_be_bytes();
                BigInt::from_bytes_be(num_bigint::Sign::Plus, &bytes)
            }
            Number::I8(n) => BigInt::from(*n),
            Number::I16(n) => BigInt::from(*n),
            Number::I32(n) => BigInt::from(*n),
            Number::I64(n) => BigInt::from(*n),
            Number::I128(n) => BigInt::from(*n),
            Number::I256(n) => {
                let bytes = n.to_be_bytes();
                BigInt::from_signed_bytes_be(&bytes)
            }
            Number::F16(n) => BigInt::from(f32::from(*n) as i64),
            Number::F32(n) => BigInt::from(*n as i64),
            Number::F64(n) => BigInt::from(*n as i64),
        }
    }

    /// Convert any numeric value to f64 (lossy for large integers).
    pub fn to_f64_lossy(&self) -> f64 {
        match self {
            Number::Int(n) => n.to_f64().unwrap_or(f64::INFINITY),
            Number::U8(n) => *n as f64,
            Number::U16(n) => *n as f64,
            Number::U32(n) => *n as f64,
            Number::U64(n) => *n as f64,
            Number::U128(n) => *n as f64,
            Number::U256(n) => n.as_f64(),
            Number::I8(n) => *n as f64,
            Number::I16(n) => *n as f64,
            Number::I32(n) => *n as f64,
            Number::I64(n) => *n as f64,
            Number::I128(n) => *n as f64,
            Number::I256(n) => n.as_f64(),
            Number::F16(n) => f64::from(f32::from(*n)),
            Number::F32(n) => *n as f64,
            Number::F64(n) => *n,
        }
    }

    /// Width within the family (None for BigInt).
    fn width(&self) -> Option<BitWidth> {
        match self {
            Number::Int(_) => None,
            Number::U8(_) | Number::I8(_) => Some(BitWidth::W8),
            Number::U16(_) | Number::I16(_) | Number::F16(_) => Some(BitWidth::W16),
            Number::U32(_) | Number::I32(_) | Number::F32(_) => Some(BitWidth::W32),
            Number::U64(_) | Number::I64(_) | Number::F64(_) => Some(BitWidth::W64),
            Number::U128(_) | Number::I128(_) => Some(BitWidth::W128),
            Number::U256(_) | Number::I256(_) => Some(BitWidth::W256),
        }
    }

    // ── Widening conversions ──────────────────────────────────────────────

    fn to_u256(&self) -> U256 {
        match self {
            Number::U8(n) => U256::from(*n),
            Number::U16(n) => U256::from(*n),
            Number::U32(n) => U256::from(*n),
            Number::U64(n) => U256::from(*n),
            Number::U128(n) => U256::from(*n),
            Number::U256(n) => *n,
            _ => unreachable!("to_u256 called on non-unsigned"),
        }
    }

    fn to_i256(&self) -> I256 {
        match self {
            Number::I8(n) => I256::from(*n),
            Number::I16(n) => I256::from(*n),
            Number::I32(n) => I256::from(*n),
            Number::I64(n) => I256::from(*n),
            Number::I128(n) => I256::from(*n),
            Number::I256(n) => *n,
            _ => unreachable!("to_i256 called on non-signed"),
        }
    }

    fn to_f64(&self) -> f64 {
        match self {
            Number::F16(n) => f64::from(f32::from(*n)),
            Number::F32(n) => *n as f64,
            Number::F64(n) => *n,
            _ => unreachable!("to_f64 called on non-float"),
        }
    }

    // ── Narrowing conversions ─────────────────────────────────────────────

    fn from_u256(val: U256, target: BitWidth) -> Result<Number, ArithError> {
        match target {
            BitWidth::W8 => u8::try_from(val)
                .map(Number::U8)
                .map_err(|_| ArithError::Overflow),
            BitWidth::W16 => u16::try_from(val)
                .map(Number::U16)
                .map_err(|_| ArithError::Overflow),
            BitWidth::W32 => u32::try_from(val)
                .map(Number::U32)
                .map_err(|_| ArithError::Overflow),
            BitWidth::W64 => u64::try_from(val)
                .map(Number::U64)
                .map_err(|_| ArithError::Overflow),
            BitWidth::W128 => u128::try_from(val)
                .map(Number::U128)
                .map_err(|_| ArithError::Overflow),
            BitWidth::W256 => Ok(Number::U256(val)),
        }
    }

    fn from_i256(val: I256, target: BitWidth) -> Result<Number, ArithError> {
        match target {
            BitWidth::W8 => i8::try_from(val)
                .map(Number::I8)
                .map_err(|_| ArithError::Overflow),
            BitWidth::W16 => i16::try_from(val)
                .map(Number::I16)
                .map_err(|_| ArithError::Overflow),
            BitWidth::W32 => i32::try_from(val)
                .map(Number::I32)
                .map_err(|_| ArithError::Overflow),
            BitWidth::W64 => i64::try_from(val)
                .map(Number::I64)
                .map_err(|_| ArithError::Overflow),
            BitWidth::W128 => i128::try_from(val)
                .map(Number::I128)
                .map_err(|_| ArithError::Overflow),
            BitWidth::W256 => Ok(Number::I256(val)),
        }
    }

    fn from_f64(val: f64, target: BitWidth) -> Number {
        match target {
            BitWidth::W16 => Number::F16(f16::from_f64(val)),
            BitWidth::W32 => Number::F32(val as f32),
            _ => Number::F64(val),
        }
    }

    /// Determine the target width when combining two values in the same family.
    fn promote_width(a: Option<BitWidth>, b: Option<BitWidth>) -> Option<BitWidth> {
        match (a, b) {
            (None, _) | (_, None) => None, // BigInt stays BigInt
            (Some(a), Some(b)) => Some(a.max(b)),
        }
    }
}

/// Arithmetic error from fixed-width operations.
#[derive(Debug, Clone)]
pub enum ArithError {
    Overflow,
    DivisionByZero,
    /// Cannot operate across numeric families.
    IncompatibleTypes {
        left: &'static str,
        right: &'static str,
    },
}

/// Perform a binary arithmetic operation on two Numbers.
pub fn arith(a: Number, b: Number, op: ArithOp) -> Result<Number, ArithError> {
    if a.family() != b.family() {
        return Err(ArithError::IncompatibleTypes {
            left: a.type_name(),
            right: b.type_name(),
        });
    }

    match a.family() {
        NumFamily::Int => {
            let (Number::Int(a), Number::Int(b)) = (&a, &b) else {
                unreachable!()
            };
            let (a, b) = (a.as_ref(), b.as_ref());
            let result = match op {
                ArithOp::Add => a + b,
                ArithOp::Sub => a - b,
                ArithOp::Mul => a * b,
                ArithOp::Div => {
                    if b.is_zero() {
                        return Err(ArithError::DivisionByZero);
                    }
                    a / b
                }
                ArithOp::Mod => {
                    if b.is_zero() {
                        return Err(ArithError::DivisionByZero);
                    }
                    a % b
                }
                ArithOp::Pow => {
                    let exp = b.to_u32().ok_or(ArithError::Overflow)?;
                    num_traits::pow::Pow::pow(a, exp)
                }
            };
            Ok(Number::Int(Arc::new(result)))
        }

        NumFamily::Unsigned => {
            let target = Number::promote_width(a.width(), b.width()).unwrap();
            let a = a.to_u256();
            let b = b.to_u256();
            let result = match op {
                ArithOp::Add => a.checked_add(b).ok_or(ArithError::Overflow)?,
                ArithOp::Sub => a.checked_sub(b).ok_or(ArithError::Overflow)?,
                ArithOp::Mul => a.checked_mul(b).ok_or(ArithError::Overflow)?,
                ArithOp::Div => {
                    if b == U256::ZERO {
                        return Err(ArithError::DivisionByZero);
                    }
                    a / b
                }
                ArithOp::Mod => {
                    if b == U256::ZERO {
                        return Err(ArithError::DivisionByZero);
                    }
                    a % b
                }
                ArithOp::Pow => {
                    let exp = u32::try_from(b).map_err(|_| ArithError::Overflow)?;
                    a.checked_pow(exp).ok_or(ArithError::Overflow)?
                }
            };
            Number::from_u256(result, target)
        }

        NumFamily::Signed => {
            let target = Number::promote_width(a.width(), b.width()).unwrap();
            let a = a.to_i256();
            let b = b.to_i256();
            let result = match op {
                ArithOp::Add => a.checked_add(b).ok_or(ArithError::Overflow)?,
                ArithOp::Sub => a.checked_sub(b).ok_or(ArithError::Overflow)?,
                ArithOp::Mul => a.checked_mul(b).ok_or(ArithError::Overflow)?,
                ArithOp::Div => {
                    if b == I256::ZERO {
                        return Err(ArithError::DivisionByZero);
                    }
                    a.checked_div(b).ok_or(ArithError::Overflow)?
                }
                ArithOp::Mod => {
                    if b == I256::ZERO {
                        return Err(ArithError::DivisionByZero);
                    }
                    a % b
                }
                ArithOp::Pow => {
                    let exp = u32::try_from(b).map_err(|_| ArithError::Overflow)?;
                    a.checked_pow(exp).ok_or(ArithError::Overflow)?
                }
            };
            Number::from_i256(result, target)
        }

        NumFamily::Float => {
            let target = Number::promote_width(a.width(), b.width()).unwrap();
            let a = a.to_f64();
            let b = b.to_f64();
            let result = match op {
                ArithOp::Add => a + b,
                ArithOp::Sub => a - b,
                ArithOp::Mul => a * b,
                ArithOp::Div => a / b, // IEEE 754: no error, produces Inf/NaN
                ArithOp::Mod => a % b,
                ArithOp::Pow => a.powf(b),
            };
            Ok(Number::from_f64(result, target))
        }
    }
}

/// Compare two Numbers. Returns None if incompatible families.
pub fn compare(a: &Number, b: &Number) -> Result<std::cmp::Ordering, ArithError> {
    if a.family() != b.family() {
        return Err(ArithError::IncompatibleTypes {
            left: a.type_name(),
            right: b.type_name(),
        });
    }

    match a.family() {
        NumFamily::Int => {
            let (Number::Int(a), Number::Int(b)) = (a, b) else {
                unreachable!()
            };
            Ok(a.as_ref().cmp(b.as_ref()))
        }
        NumFamily::Unsigned => Ok(a.to_u256().cmp(&b.to_u256())),
        NumFamily::Signed => Ok(a.to_i256().cmp(&b.to_i256())),
        NumFamily::Float => {
            let a = a.to_f64();
            let b = b.to_f64();
            a.partial_cmp(&b).ok_or(ArithError::IncompatibleTypes {
                left: "NaN",
                right: "NaN",
            })
        }
    }
}

/// Arithmetic operations.
#[derive(Debug, Clone, Copy)]
pub enum ArithOp {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Pow,
}

impl fmt::Display for Number {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Number::Int(n) => write!(f, "{n}"),
            Number::U8(n) => write!(f, "{n}u8"),
            Number::U16(n) => write!(f, "{n}u16"),
            Number::U32(n) => write!(f, "{n}u32"),
            Number::U64(n) => write!(f, "{n}u64"),
            Number::U128(n) => write!(f, "{n}u128"),
            Number::U256(n) => write!(f, "{n}u256"),
            Number::I8(n) => write!(f, "{n}i8"),
            Number::I16(n) => write!(f, "{n}i16"),
            Number::I32(n) => write!(f, "{n}i32"),
            Number::I64(n) => write!(f, "{n}i64"),
            Number::I128(n) => write!(f, "{n}i128"),
            Number::I256(n) => write!(f, "{n}i256"),
            Number::F16(n) => write!(f, "{n}f16"),
            Number::F32(n) => {
                if n.fract() == 0.0 && n.is_finite() {
                    write!(f, "{n:.1}f32")
                } else {
                    write!(f, "{n}f32")
                }
            }
            Number::F64(n) => {
                // f64 is the default float — no suffix, like Int has no suffix.
                if n.fract() == 0.0 && n.is_finite() {
                    write!(f, "{n:.1}")
                } else {
                    write!(f, "{n}")
                }
            }
        }
    }
}

impl PartialEq for Number {
    fn eq(&self, other: &Self) -> bool {
        if self.family() != other.family() {
            return false;
        }
        match self.family() {
            NumFamily::Int => {
                let (Number::Int(a), Number::Int(b)) = (self, other) else {
                    unreachable!()
                };
                Arc::ptr_eq(a, b) || a == b
            }
            NumFamily::Unsigned => self.to_u256() == other.to_u256(),
            NumFamily::Signed => self.to_i256() == other.to_i256(),
            NumFamily::Float => self.to_f64() == other.to_f64(),
        }
    }
}

impl Eq for Number {}
