# ezc

A stack-based programming language with arbitrary-precision integers, 16
numeric types, and first-class editor tooling. Code is a flat sequence of
values and operators in reverse Polish notation.

```ezc
# sum of squares from 1 to 10
10 iota (sq) map sum
```

No ceremony. No types to declare. Just push values, apply operators.

## Why ezc?

- **Big integers by default** — `int` is arbitrary-precision. Fixed-width
  types (`u8`–`u256`, `i8`–`i256`, `f16`–`f64`) are opt-in.
- **Mathematical focus** — clean operator algebra, type-family promotion,
  no surprises with overflow.
- **First-class tooling** — debugger (DAP), language server (LSP), and
  extensions for VS Code, Neovim, Zed, and Helix.
- **Runs in the browser** — every code block on this site is runnable.

## Where to start

- [**Getting started**](getting-started.md) — install and run your first program
- [**Stack basics**](tutorial/basics.md) — learn the language from the ground up
- [**Operators**](reference/operators.md) — the full reference
- [**Playground**](playground.md) — try it now, no install needed
