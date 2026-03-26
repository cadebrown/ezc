# ezc Language Reference

## Overview

ezc is a stack-based language using reverse Polish notation (RPN). Programs are
sequences of values and operators. Values are pushed onto the stack; operators
pop their arguments and push their results.

There is no separate boolean type — comparisons push `1` (truthy) or `0` (falsy).

## Types

### Numeric Types

| Type | Description | Literal |
|------|-------------|---------|
| `int` | Arbitrary-precision big integer (default) | `42`, `0xFF` |
| `u8`..`u256` | Unsigned fixed-width integer | `42u8`, `0xFFu16` |
| `i8`..`i256` | Signed fixed-width integer | `42i32`, `0xFFi64` |
| `f16`, `f32`, `f64` | Floating-point | `3.14`, `3.14f32`, `42f64` |

Bare integer literals are `int` (arbitrary precision). Add a suffix for typed:

```
42          # int (BigInt)
42u8        # u8
0xFF        # int (255)
0xFFu32     # u32
3.14        # f64
3.14f32     # f32
```

Arithmetic within a family promotes to the wider type:
- `3u8 4u16 +` → `7u16`
- `1.0f32 2.0f64 +` → `3.0f64`

Cross-family arithmetic is a type error (`u8 + i8`, `int + u8`, `f64 + int`).
Use type constructors to convert: `42 f64 3.14 +`.

### Strings

Immutable, interned. Created with double quotes:

```
"hello"             # push string
"line\nbreak"       # escape sequences: \n \t \r \\ \" \0
"hello" " world" |  # concatenate → "hello world"
```

### Binary Blobs

Immutable byte arrays. Created via conversion:

```
"hello" bin         # string → bytes
```

### Blocks

Deferred code, created with `(...)`. Not evaluated until `!`:

```
(2 *)               # push a block that doubles the top
3 (4 +) !           # → 7
(1 +) (2 *) |       # compose blocks → block that does 1+ then 2*
```

Blocks use dynamic scoping — they see variables that exist at execution time.

### Lists

Eagerly evaluated collections, created with `[...]`:

```
[1 2 3]             # → [1 2 3]
[3 4 +]             # → [7]  (arithmetic is evaluated)
[1 2 3 4 +]         # → [1 2 7]
```

### Scoped Blocks

Created with `{...}`. Evaluates immediately with a local scope — variable
bindings inside don't leak out, but stack effects pass through:

```
{ 10 @temp $temp 2 * }  # → 20 ($temp not visible after)
1 @x { 2 @x $x } $x    # → 2 1 (inner shadows, outer restored)
```

## Variables

`@name` pops the top of the stack and binds it to a variable.
`$name` pushes the variable's value onto the stack.

```
5 @x $x $x +        # → 10
(: *) @square        # define a function
5 $square !          # → 25
```

Bindings at the top level persist for the session/file. Use `{...}` to limit scope.

## Operators

### Arithmetic

All binary. Pop two values, push result.

| Op | Description |
|----|-------------|
| `+` | Addition |
| `-` | Subtraction |
| `*` | Multiplication |
| `/` | Division (integer for ints, IEEE for floats) |
| `%` | Modulo |
| `^` | Exponentiation |

### Stack Manipulation

| Op | Description | Example |
|----|-------------|---------|
| `:` | Dup (copy top) | `5 :` → `5 5` |
| `~` | Swap top two | `1 2 ~` → `2 1` |
| `_` | Over (copy second to top) | `1 2 _` → `1 2 1` |

### Control Flow

| Op | Description | Example |
|----|-------------|---------|
| `!` | Execute a block | `3 (4 +) !` → `7` |
| `?` | Conditional: if top is falsy, also pop next | `5 0 ?` → (empty) |
| `??` | Ternary: pop cond, keep one of two values | `10 20 1 ??` → `20` |
| `&` | Loop: `cond body &` — while cond is truthy | `5 (: 0 !=) (1 -) &` → `0` |

### Comparison

Pop two values, push `1` (true) or `0` (false). Works on numbers and strings.

| Op | Description | Example |
|----|-------------|---------|
| `==` | Equal | `3 3 ==` → `1` |
| `!=` | Not equal | `3 4 !=` → `1` |
| `<` | Less than | `3 4 <` → `1` |
| `>` | Greater than | `4 3 >` → `1` |
| `<=` | Less or equal | `3 3 <=` → `1` |
| `>=` | Greater or equal | `4 3 >=` → `1` |

### Compose / Concatenate

`|` works on matching container types:

| Types | Effect |
|-------|--------|
| `list \| list` | Concatenate lists |
| `str \| str` | Concatenate strings |
| `block \| block` | Compose blocks (body of a, then body of b) |

```
[1 2] [3 4] |           # → [1 2 3 4]
"hello" " world" |      # → "hello world"
(1 +) (2 *) | !         # equivalent to (1 + 2 *)
```

### Higher-Order Operations

| Op | Description | Example |
|----|-------------|---------|
| `&!` | Map: apply block to each element | `[1 2 3] (1 +) &!` → `[2 3 4]` |
| `&?` | Filter: keep elements where block is truthy | `[1 2 3 4 5] (3 >) &?` → `[4 5]` |

### Type Constructors

Bare type names pop a value and push the converted result:

| Name | Description |
|------|-------------|
| `int` | Convert to arbitrary-precision integer |
| `u8`..`u256` | Convert to unsigned fixed-width |
| `i8`..`i256` | Convert to signed fixed-width |
| `f16`, `f32`, `f64` | Convert to float |
| `str` | Convert to string |
| `bin` | Convert to binary blob |

```
42 f32               # → 42.0f32
3.14 int             # → 3 (truncates)
42 str               # → "42"
"hello" bin          # → 0x68656c6c6f
```

## Comments

Line comments start with `#`:

```
3 4 +   # this is a comment
```

## Truthiness

Used by `?`, `??`, `&`, and `&?`:

| Type | Falsy | Truthy |
|------|-------|--------|
| Any number | `0` | Non-zero |
| `str` | `""` (empty) | Non-empty |
| `bin` | Empty | Non-empty |
| `list` | `[]` (empty) | Non-empty |
| `block` | Never | Always |

## Error Handling

Errors include source annotations pointing at the operator and its operands:

```
Error: `+` got list and int — needs two numbers
   ╭─[ file.ezc:1:11 ]
   │
 1 │ [1 2 3] 4 +
   │ ───┬─── ┬ ┬
   │    │    ╰──── this is int
   │    ╰───────── this is list
   │           ╰── `+` got list and int — needs two numbers
───╯
```

## Examples

```
# Factorial via loop: 5! = 120
5 1 ~ (~ : 0 !=) (~ _ * ~ 1 -) & ~
# Stack: 120 0 → drop the 0, keep 120

# Fibonacci sequence as a list
[1 1] 10 (: (: 1 - 0 !=) ~ (: _ + ~) & ~) &!
# ... work in progress

# Named functions
(: *) @square
(: $square ! $square !) @fourth
2 $fourth !    # → 16

# Map with a variable
10 @offset
[1 2 3] ($offset +) &!    # → [11 12 13]
```
