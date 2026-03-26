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

# lists, map, filter
[1 2 3 4 5] (2 *) &!
# → [2 4 6 8 10]
```

## Features

- **Big integers** as the default numeric type
- **RPN syntax** -- no precedence rules, no infix grouping
- **First-class blocks** -- `(...)` captures code for later execution with `!`
- **Eager lists** -- `[...]` evaluates and collects into a list
- **Functional combinators** -- `&!` (map), `&?` (filter), `|` (compose), `&` (loop)
- **Comparison operators** -- `==` `!=` `<` `>` `<=` `>=`
- **Stack ops** -- `:` (dup), `~` (swap), `_` (over)

## Getting Started

```bash
# Build
cargo build

# Run a program
cargo run -p ezc-cli -- run examples/hello.ezc

# Interactive REPL
cargo run -p ezc-cli -- repl

# Check syntax without running
cargo run -p ezc-cli -- check program.ezc
```

## Project Structure

| Crate | Description |
|-------|-------------|
| `crates/ezc` | Core language: types, parser, evaluator |
| `crates/ezc-cli` | CLI binary (`ezc run`, `ezc check`, `ezc repl`) |
| `crates/ezc-lsp` | Language server (planned) |
| `crates/ezc-web` | WASM embedding (planned) |

## Language Reference

See [`docs/language.md`](docs/language.md) for the full language reference.

## License

MIT
