//! Value interner for strings, binary blobs, and big integers.
//!
//! Each [`Engine`](crate::eval::Engine) owns an [`Interner`] that deduplicates
//! immutable heap values. Interned values share the same [`Arc`] allocation,
//! so equality checks can short-circuit via [`Arc::ptr_eq`].
//!
//! Only immutable, heap-allocated types benefit from interning. Fixed-width
//! numerics (u8..u256, floats) live on the stack and compare directly.

use std::collections::HashSet;
use std::sync::Arc;

use num_bigint::BigInt;

use crate::number::Number;
use crate::types::{EzBin, EzStr};

/// Per-engine value interner.
///
/// Deduplicates immutable values so that structurally identical values share
/// the same `Arc` allocation. This gives two benefits:
///
/// 1. **Fast equality**: `Arc::ptr_eq` is O(1) — no content comparison needed
///    for values from the same interner.
/// 2. **Memory savings**: repeated identical values (e.g., the same string
///    appearing in a loop) share one allocation.
#[derive(Debug, Default)]
pub struct Interner {
    interned_strs: HashSet<Arc<str>>,
    interned_bins: HashSet<Arc<[u8]>>,
    interned_ints: HashSet<Arc<BigInt>>,
}

impl Interner {
    pub fn new() -> Self {
        Self::default()
    }

    /// Intern a string, returning the canonical [`EzStr`].
    pub fn intern_str(&mut self, s: &str) -> EzStr {
        if let Some(existing) = self.interned_strs.get(s) {
            return EzStr(existing.clone());
        }
        let arc: Arc<str> = Arc::from(s);
        self.interned_strs.insert(arc.clone());
        EzStr(arc)
    }

    /// Intern a binary blob, returning the canonical [`EzBin`].
    pub fn intern_bin(&mut self, data: &[u8]) -> EzBin {
        if let Some(existing) = self.interned_bins.get(data) {
            return EzBin(existing.clone());
        }
        let arc: Arc<[u8]> = Arc::from(data);
        self.interned_bins.insert(arc.clone());
        EzBin(arc)
    }

    /// Intern a big integer, returning `Number::Int` with the canonical `Arc`.
    pub fn intern_int(&mut self, n: BigInt) -> Number {
        if let Some(existing) = self.interned_ints.get(&n) {
            return Number::Int(existing.clone());
        }
        let arc = Arc::new(n);
        self.interned_ints.insert(arc.clone());
        Number::Int(arc)
    }

    /// Total number of interned values across all pools.
    pub fn len(&self) -> usize {
        self.interned_strs.len() + self.interned_bins.len() + self.interned_ints.len()
    }

    /// Whether all pools are empty.
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn str_dedup() {
        let mut interner = Interner::new();
        let a = interner.intern_str("hello");
        let b = interner.intern_str("hello");
        assert!(Arc::ptr_eq(&a.0, &b.0));

        let c = interner.intern_str("world");
        assert!(!Arc::ptr_eq(&a.0, &c.0));
    }

    #[test]
    fn bin_dedup() {
        let mut interner = Interner::new();
        let a = interner.intern_bin(&[1, 2, 3]);
        let b = interner.intern_bin(&[1, 2, 3]);
        assert!(Arc::ptr_eq(&a.0, &b.0));
    }

    #[test]
    fn int_dedup() {
        let mut interner = Interner::new();
        let a = interner.intern_int(BigInt::from(42));
        let b = interner.intern_int(BigInt::from(42));
        match (&a, &b) {
            (Number::Int(a), Number::Int(b)) => assert!(Arc::ptr_eq(a, b)),
            _ => panic!("expected Int"),
        }
    }

    #[test]
    fn len_tracks_all_pools() {
        let mut interner = Interner::new();
        assert!(interner.is_empty());

        interner.intern_str("a");
        interner.intern_bin(&[1]);
        interner.intern_int(BigInt::from(1));
        assert_eq!(interner.len(), 3);

        // Duplicate — count shouldn't increase.
        interner.intern_str("a");
        assert_eq!(interner.len(), 3);
    }
}
