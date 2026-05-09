# Operators

Every operator in ezc, organized by category. The "stack effect" column
uses the convention `before → after`, listing the items closest to the
top of the stack rightmost.

## Arithmetic

| op  | stack effect    | description |
|-----|-----------------|-------------|
| `+` | `a b → (a+b)`   | add (within numeric family, promotes) |
| `-` | `a b → (a-b)`   | subtract |
| `*` | `a b → (a*b)`   | multiply |
| `/` | `a b → (a/b)`   | divide (integer for ints, float for floats) |
| `%` | `a b → (a%b)`   | modulo |
| `^` | `a b → (a^b)`   | power |

```ezc
3 4 +
```

## Comparison

All push `1` for true, `0` for false. There is no separate boolean type.

| op   | stack effect | description |
|------|--------------|-------------|
| `==` | `a b → bool` | equal |
| `!=` | `a b → bool` | not equal |
| `<`  | `a b → bool` | less than |
| `>`  | `a b → bool` | greater than |
| `<=` | `a b → bool` | less than or equal |
| `>=` | `a b → bool` | greater than or equal |

## Stack manipulation

| op  | stack effect    | name |
|-----|-----------------|------|
| `,` | `a → a a`       | dup |
| `;` | `a →`           | drop |
| `~` | `a b → b a`     | swap |
| `_` | `a b → a b a`   | over |

## I/O

| op   | stack effect | description |
|------|--------------|-------------|
| `:`  | `a →`        | write line (any type) |
| `.`  | `→ str`      | read line |
| `wl` | `str →`      | write line (string) |
| `rl` | `→ str`      | read line (alias of `.`) |
| `wb` | `int →`      | write a single byte |
| `rb` | `→ int`      | read a single byte |

## Control flow

| op   | stack effect              | description |
|------|---------------------------|-------------|
| `!`  | varies                    | execute: pops a block/list/string and runs/splats/evals |
| `?`  | `cond block →`            | conditional execute (only runs block if cond truthy) |
| `??` | `cond then else → result` | ternary: pick one branch by truthiness |
| `&`  | `cond-block body-block →` | while loop |
| `&!` | `list block → list`       | map |
| `&?` | `list block → list`       | filter |
| `&/` | `list init block → acc`   | fold (left-to-right) |

```ezc
1 (10 :) ?     # prints 10 because cond was truthy
0 (10 :) ?     # prints nothing
1 (10) (20) ?? # → 10
```

## Compose

| op  | stack effect      | description |
|-----|-------------------|-------------|
| `\|` | `a b → (a\|b)`    | concatenate strings, lists, or blocks |

```ezc
"foo" "bar" |          # → "foobar"
[1 2] [3 4] |          # → [1 2 3 4]
```

## Variables

| form    | description |
|---------|-------------|
| `@name` | bind: pop top of stack, store in name |
| `$name` | recall: push value of name |
| `name`  | bare: recall, but auto-execute if value is a block |

## Containers

| syntax  | description |
|---------|-------------|
| `(...)` | block — deferred code, executed by `!` |
| `[...]` | list — eager evaluation, gathers leftovers |
| `{...}` | scope — evaluates immediately with local bindings |

## Stack introspection

| op      | stack effect | description |
|---------|--------------|-------------|
| `depth` | `→ int`      | current stack height |
| `clear` | varies       | drop everything on the stack |
| `words` | `→ list`     | list all currently-defined names |
