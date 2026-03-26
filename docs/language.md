# ezc Language Reference

## Overview

ezc is a stack-based language using reverse Polish notation (RPN). Programs are
sequences of values and operators. Values are pushed onto the stack; operators
pop their arguments and push their results.

## Values

### Integers

The default numeric type. Arbitrary precision (big integers).

```
42          # push 42
0           # push 0
123456789   # push 123456789
```

### Booleans

Produced by comparison operators. `true` and `false`.

### Lists

Created with `[...]`. The contents are evaluated eagerly on a sub-stack,
and the resulting stack becomes the list.

```
[1 2 3]         # → [1, 2, 3]
[3 4 +]         # → [7]  (arithmetic is evaluated)
[1 2 3 4 + ]    # → [1, 2, 7]
```

### Blocks

Created with `(...)`. The contents are *not* evaluated — they are captured
as a lambda for later execution with `!`.

```
(2 *)           # push a block that doubles the top of stack
3 (4 +) !       # → 7
```

## Operators

### Arithmetic

All binary operators pop two values and push the result.

| Op | Description |
|----|-------------|
| `+` | Addition |
| `-` | Subtraction |
| `*` | Multiplication |
| `/` | Integer division |
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
| `&` | Loop: `cond_block body_block &` — runs cond inline (must leave one value); loops while truthy | `5 (: 0 !=) (1 -) &` → `0` |

### Comparison

All comparison operators pop two values and push a `Bool`.
Ordering operators (`<`, `>`, `<=`, `>=`) require integers; `==` and `!=` work on any values.

| Op | Description | Example |
|----|-------------|---------|
| `==` | Equal | `3 3 ==` → `true` |
| `!=` | Not equal | `3 4 !=` → `true` |
| `<` | Less than | `3 4 <` → `true` |
| `>` | Greater than | `4 3 >` → `true` |
| `<=` | Less than or equal | `3 3 <=` → `true` |
| `>=` | Greater than or equal | `4 3 >=` → `true` |

### List Operations

| Op | Description | Example |
|----|-------------|---------|
| `\|` | Concatenate two lists | `[1 2] [3 4] \|` → `[1 2 3 4]` |
| `&!` | Map: apply block to each element | `[1 2 3] (1 +) &!` → `[2 3 4]` |
| `&?` | Filter: keep elements where block is truthy | `[1 2 3] (2 >) &?` → `[3]` |

### Reserved

`$`, `@` — semantics to be determined.

## Comments

Line comments start with `#` and extend to end of line.

```
3 4 +   # this is a comment
```

## Truthiness

- `Int`: truthy if non-zero
- `Bool`: truthy if `true`
- `Block`: always truthy
- `List`: truthy if non-empty
