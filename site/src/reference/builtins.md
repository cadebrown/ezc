# Builtins

Built-in named operations, beyond the symbolic operators.

## Collection

| name     | stack effect              | description |
|----------|---------------------------|-------------|
| `len`    | `coll → int`              | length of list / string / binary |
| `nth`    | `coll i → elem`           | zero-indexed access |
| `tl`     | `list → list`             | tail (drop first) |
| `rev`    | `list → list` / `str → str` | reverse |
| `srt`    | `list → list`             | sort |
| `take`   | `coll n → coll`           | first n elements |
| `skip`   | `coll n → coll`           | drop first n elements |
| `zip`    | `lhs rhs → list`          | pair-wise zip into list of pairs |
| `range`  | `lo hi → list`            | integer range `[lo .. hi-1]` |
| `cut`    | `str delim → list`        | split string by delimiter |
| `cat`    | `list delim → str`        | join list with delimiter |
| `flat`   | `list → list`             | flatten one level (prelude) |

## Higher-order (named aliases)

| name   | symbol | description |
|--------|--------|-------------|
| `each` | —      | iterate over a list or count |
| `map`  | `&!`   | apply block to each element |
| `fil`  | `&?`   | filter |
| `red`  | `&/`   | fold/reduce |
| `loop` | `&`    | while loop |
| `if`   | `?`    | conditional |
| `ifel` | `??`   | ternary |

## Reflection

| name     | stack effect | description |
|----------|--------------|-------------|
| `typeof` | `a → str`    | type name string |
| `words`  | `→ list`     | all currently-defined names |
| `depth`  | `→ int`      | current stack height |
| `clear`  | `→`          | drop all stack values |

## Type constructors

Every type name is a constructor. They convert the top-of-stack value to
the given type, or fail with a useful error.

```text
int                  str         bin
u8 u16 u32 u64 u128 u256
i8 i16 i32 i64 i128 i256
f16 f32 f64
```

```ezc
42 f64       # → 42.0
3.7 int      # → 3      (truncates)
42 str       # → "42"   (number to decimal string)
```

## Modules

| name     | stack effect | description |
|----------|--------------|-------------|
| `import` | `path →`     | load and evaluate a file |
