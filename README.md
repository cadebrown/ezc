# ezc

A stack-based, reverse-polish-notation programming language with a mathematical focus.

```
# compute (3 + 4) * 2
3 4 + 2 *
# → 14

# big integers are the default
2 100 ^
# → 1267650600228229401496703205376

# blocks and execution
3 (4 +) !
# → 7

# strings
"hello" " world" |
# → "hello world"

# variables and named functions
(: *) @square
5 $square !
# → 25

# typed numerics
42u8 3u8 +          # → 45u8
3.14f32             # → 3.14f32
0xFF                # → 255
42 f64              # → 42.0f64

# lists, map, filter
[1 2 3 4 5] (2 *) &!
# → [2 4 6 8 10]

[1 2 3 4 5] (3 >) &?
# → [4 5]
```

## Features

- **Big integers** as the default numeric type — arbitrary precision
- **16 numeric types** — `int`, `u8`-`u256`, `i8`-`i256`, `f16`/`f32`/`f64`
- **Strings and binary blobs** — immutable, interned
- **RPN syntax** — no precedence rules, no parentheses for grouping
- **First-class blocks** — `(...)` captures code for later execution with `!`
- **Block composition** — `(a) (b) |` composes into `(a b)`
- **Variables** — `@name` binds, `$name` recalls, `{...}` scopes
- **Eager lists** — `[...]` evaluates and collects
- **Functional combinators** — `&!` (map), `&?` (filter), `&` (loop)
- **Type constructors** — bare words (`f32`, `str`, `bin`) as conversion functions
- **Rich errors** — ariadne reports with multi-label source annotations
- **TUI REPL** — syntax highlighting, tab completion, stack trace, history hints

## Getting Started

```bash
# Build
cargo build

# Run a program
cargo run -p ezc-cli -- run program.ezc

# Interactive TUI REPL
cargo run -p ezc-cli -- repl

# Plain terminal REPL (reedline)
cargo run -p ezc-cli -- repl --no-tui

# Check syntax without running
cargo run -p ezc-cli -- check program.ezc
```

## Project Structure

| Crate | Description |
|-------|-------------|
| `crates/ezc` | Core language: types, parser, evaluator, interner |
| `crates/ezc-cli` | CLI binary with TUI and reedline REPLs |
| `crates/ezc-lsp` | Language server (planned) |
| `crates/ezc-web` | WASM embedding (planned) |

## Language Reference

See [`docs/language.md`](docs/language.md) for the full language reference.

## License

MIT
