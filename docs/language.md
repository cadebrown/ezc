# EZC Language Guide

EZC is concatenative, postfix, and stack-based.

- Data first, operators after (`RPN`).
- All execution happens on a value stack.
- `docs/` is the language source of truth.

---

## Comments

Use `#` for line comments.

```ezc run
2 3 + # this is ignored
4 *
```

## Core Arithmetic

```ezc run
2 3 + 4 *
```

`+ - * / %` consume operands from the stack.

## Text Objects

EZC supports both single-quoted and double-quoted text literals.

```ezc run
"hello, world" prt
```

```ezc run
'single quoted text' prt
```

## Delayed Execution Blocks

`( ... )` creates a delayed code block as one stack value.

```ezc run
(5 dup * prt)
```

Execute a block with `!`.

```ezc run
(5 dup * prt)!
```

## Sub-stacks

`[ ... ]` creates a composite stack object.

```ezc run
[a 1 [b 2] (9 1 -)]
```

## Conditional `?`

`a b c ?` chooses between `a` and `b` using `c`.

- if `c` is truthy, result is `a`
- if `c` is falsy, result is `b`

```ezc run
111 222 1 ?
```

```ezc run
111 222 0 ?
```

## Loop `^`

`^` expects a block and executes it repeatedly until the block leaves a falsy element on top, which `^` then consumes as the stop condition.

```ezc run
3 (dup prt 1 - dup) ^
```

## Stack + Logic Words

- stack: `dup del swp ovr`
- compare: `= < >`
- logic: `& | not`
- print: `prt`
- control: `! ? ^`

Terse symbolic aliases:

- `,` -> `dup`
- `.` -> `del`
- `_` -> `ovr`
- `~` -> `swp`

## CLI

```bash
cargo run -- run demo/square.ezc
cargo run -- run demo/hello.ezc
cargo run -- run demo/fib.ezc
cargo run -- run demo/gcd.ezc
cargo run -- run demo/factorial.ezc
cargo run -- run demo/powers_of_two.ezc
cargo run -- run demo/triangular.ezc
cargo run -- disasm demo/loop_countdown.ezc
cargo run -- check demo/substack.ezc
cargo run -- eval "(5 dup * prt)!"
cargo run -- repl
```

REPL commands:

- `:help` show command list
- `:clear` clear transcript
- `:quit` exit

Verbose intermediate pipeline output:

```bash
cargo run -- --verbose run demo/loop_countdown.ezc
```
