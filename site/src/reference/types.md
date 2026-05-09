# Type system

ezc is dynamically typed. Every value carries its type at runtime and
operators check types when popping.

## Value types

| type    | description |
|---------|-------------|
| `int`   | arbitrary-precision integer (BigInt) |
| `u8`–`u256` | fixed-width unsigned (8/16/32/64/128/256-bit) |
| `i8`–`i256` | fixed-width signed |
| `f16`/`f32`/`f64` | IEEE-754 floating point |
| `str`   | immutable, interned UTF-8 string |
| `bin`   | immutable, interned byte buffer |
| `list`  | heterogeneous sequence of values |
| `block` | deferred code (sequence of expressions) |

## Numeric families

Arithmetic operators only work between values in the same family:

- **Integer family** — `int`
- **Unsigned family** — `u8`, `u16`, ..., `u256`
- **Signed family** — `i8`, `i16`, ..., `i256`
- **Float family** — `f16`, `f32`, `f64`

Within a family, operations promote to the wider type:

```ezc
3u8 4u32 +    # → 7u32 (u8 promoted)
```

Cross-family is an error — `int`, unsigned, signed, and float are
*separate* families. Convert explicitly:

```ezc
3 4u8 +        # error: int and u8 are different families
3 u8 4u8 +     # → 7u8  (convert 3 to u8 first)
```

`int` (BigInt) is its own family. Within int, all ops are exact:

```ezc
2 100 ^        # 2^100 as int (no overflow)
```

Fixed-width types overflow (and error) when results exceed their range:

```ezc
2u8 7u8 ^      # 128u8 — fits
2u8 8u8 ^      # error: overflow in u8
```

## Truthiness

Used by `?`, `??`, `&` and the higher-order ops:

| type   | falsy when |
|--------|------------|
| number | zero |
| `str`  | empty |
| `bin`  | empty |
| `list` | empty |
| `block` | never (always truthy) |

## Interning

`str`, `bin`, and `int` (BigInt) values are interned per engine instance.
Equality is constant-time pointer comparison; copying is shared via
`Arc`. This makes large strings and integers cheap to pass around.

## Spans

Every value on the stack carries a source span — the byte range of the
expression that pushed it. The DAP debugger and ariadne error reports
use these spans to highlight the *exact* token responsible for a
mismatch, not just a line number.
