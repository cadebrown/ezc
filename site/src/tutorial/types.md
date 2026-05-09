# Numeric types

ezc has 16 numeric types organized into three families. The default,
unsuffixed integer type is **arbitrary-precision** — no silent overflow.

## The default: `int`

A bare integer literal is `int` — backed by `BigInt`:

```ezc
2 100 ^      # → 2¹⁰⁰  (a 31-digit number, exact)
```

Try the same with a fixed-width type and you'll see overflow:

```ezc
2u64 100u64 ^   # error: overflow
```

## Three families

| Family   | Types                            |
|----------|----------------------------------|
| Integer  | `int` (BigInt — default)         |
| Unsigned | `u8`, `u16`, `u32`, `u64`, `u128`, `u256` |
| Signed   | `i8`, `i16`, `i32`, `i64`, `i128`, `i256` |
| Float    | `f16`, `f32`, `f64`              |

## Typed literals

Suffix a literal with a type name:

```ezc
42u8       # u8
42i64      # i64
3.14f32    # f32
0xFFu16    # hex literal as u16
```

## Type constructors

Type names are also conversion functions. `42 f32` means "push 42, then
convert it to f32":

```ezc
42 f32      # → 42.0f32
3.7 int     # → 3   (truncates)
"123" int   # → 123 (parse)
```

## Arithmetic promotion

Operations within the same family promote to the wider type:

```ezc
3u8 4u32 +     # → 7u32  (u8 promoted to u32)
3 4u32 +       # → 7u32  (int promoted to u32)
```

Cross-family arithmetic is an error. You must convert explicitly:

```ezc
3 4.0 +        # error: int + f64
3 f64 4.0 +    # → 7.0f64
```

## `typeof`

```ezc
42 typeof          # → "int"
42u8 typeof        # → "u8"
3.14 typeof        # → "f64"
"hello" typeof     # → "str"
[1 2] typeof       # → "list"
(+) typeof         # → "block"
```

## What's next

- [**I/O and modules**](io.md) — read, write, import
