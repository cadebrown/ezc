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
(, *) @square        # define a function
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
| `,` | Dup (copy top) | `5 ,` → `5 5` |
| `;` | Drop (discard top) | `1 2 3 ;` → `1 2` |
| `~` | Swap top two | `1 2 ~` → `2 1` |
| `_` | Over (copy second to top) | `1 2 _` → `1 2 1` |

### Control Flow

| Op | Description | Example |
|----|-------------|---------|
| `!` | Execute block / splat list / eval string | `3 (4 +) !` → `7` |
| `?` | Conditional execute: pop cond, if truthy execute next (block) | `(2 *) 1 ?` → executes `2 *` |
| `??` | Ternary: pop cond, keep one of two values | `10 20 1 ??` → `20` |
| `&` | Loop: `cond body &` — while cond is truthy | `5 (, 0 !=) (1 -) &` → `0` |

`!` works on multiple types:
- **Block**: execute the code — `(3 4 +) !` → `7`
- **List**: splat onto stack — `[1 2 3] !` → `1 2 3`
- **String**: eval as code — `"3 4 +" !` → `7`

### I/O

| Op | Description |
|----|-------------|
| `:` | Write: print top of stack with newline, consume it |
| `.` | Read: read a line from stdin, push as string |
| `rl` | Read line (same as `.`) |
| `wl` | Write line with newline (same as `:`) |
| `rb` | Read byte, push as int |
| `wb` | Pop int, write as byte |

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
| `&/` | Fold: `list init (block) &/` | `[1 2 3] 0 (+) &/` → `6` |

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
# Named functions
(, *) @square
(, $square ! $square !) @fourth
2 $fourth !    # → 16

# Sum and product via fold
[1 2 3 4 5] 0 (+) &/    # → 15
[1 2 3 4 5] 1 (*) &/    # → 120

# Map with a captured variable
10 @offset
[1 2 3] ($offset +) &!    # → [11 12 13]

# Splat a list onto the stack
[10 20 30] ! + +           # → 60

# Eval a string as code
"3 4 +" !                  # → 7

# I/O
"What is your name?" :     # prompt (`:` prints with newline)
. @name                    # read a line, bind to name
"Hello, " $name | :       # greet
```

## Standard Library (Prelude)

The prelude (`std/prelude.ezc`) is loaded automatically before every program and
REPL session. It defines the following names as regular variable bindings:

### Control Flow

| Name | Stack Effect | Description |
|------|-------------|-------------|
| `if` | `(block) cond if` | Execute block if cond is truthy (alias for `?`) |
| `ifel` | `(else) (then) cond ifel` | If-else (alias for `??`) |

### Predicates

| Name | Stack Effect | Description |
|------|-------------|-------------|
| `dvb` | `a b dvb` | 1 if `a` divisible by `b` |
| `zero` | `a zero` | 1 if `a` is zero |
| `even` | `a even` | 1 if `a` is even |
| `odd` | `a odd` | 1 if `a` is odd |
| `ltz` | `a ltz` | 1 if `a < 0` |
| `gtz` | `a gtz` | 1 if `a > 0` |

### Logic

| Name | Stack Effect | Description |
|------|-------------|-------------|
| `not` | `a not` | Logical negation |
| `and` | `a b and` | 1 if both truthy |
| `or` | `a b or` | 1 if either truthy |

### Stack Combinators

| Name | Stack Effect | Description |
|------|-------------|-------------|
| `dup` | `a dup` | Copy top (named `,`) |
| `drop` | `a drop` | Discard top (named `;`) |
| `swap` | `a b swap` | Swap top two (named `~`) |
| `over` | `a b over` | Copy second (named `_`) |
| `nip` | `a b nip` | Drop second |
| `tuck` | `a b tuck` | Copy top under second |
| `dup2` | `a b dup2` | Copy top two |
| `rot` | `a b c rot` | Rotate: `b c a` |
| `id` | | No-op |
| `dfl` | `val fallback dfl` | Use fallback if val is falsy |
| `inc` | `a inc` | `a + 1` |
| `dec` | `a dec` | `a - 1` |

### Math

| Name | Stack Effect | Description |
|------|-------------|-------------|
| `sq` | `a sq` | Square (`a * a`) |
| `neg` | `a neg` | Negate (`0 - a`) |
| `abs` | `a abs` | Absolute value |
| `sum` | `[list] sum` | Sum of list |
| `prod` | `[list] prod` | Product of list |
| `iota` | `n iota` | `[0 1 ... n-1]` |

### Collections

| Name | Stack Effect | Description |
|------|-------------|-------------|
| `hd` | `[list] hd` | First element |
| `flat` | `[[nested]] flat` | Flatten one level |
| `apply` | `[args] (block) apply` | Splat args, exec block |

### Comparison

| Name | Stack Effect | Description |
|------|-------------|-------------|
| `min` | `a b min` | Smaller of two values |
| `max` | `a b max` | Larger of two values |

### Printing

| Name | Stack Effect | Description |
|------|-------------|-------------|
| `peek` | `a peek` | Print without consuming |

## Builtin Shadowing

Prelude names are ordinary variable bindings, not keywords. You can rebind any
of them:

```
(, , * ~ * +) @sq    # redefine sq to compute a^2 differently
```

Builtins like `+`, `-`, `!`, `?` etc. are *not* variable bindings and cannot
be shadowed. Type constructors (`int`, `f64`, `str`, etc.) and collection
operations (`len`, `nth`, `rev`, etc.) are ident-builtins resolved by the
evaluator; rebinding them with `@` will shadow them for `$name !` usage but
the bare name still invokes the builtin.
