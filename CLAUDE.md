# CLAUDE.md -- ezc language project

## Project Overview

ezc is a stack-based, reverse-polish-notation programming language with a mathematical focus.
Big integers are the default numeric type. Code is a flat sequence of values and operators.
No separate boolean type — comparisons push `1`/`0`.

## Project Structure

Rust workspace with 4 crates under `crates/`:

- `ezc` -- core language (types, lexer, parser, evaluator, interner)
- `ezc-cli` -- CLI binary (`ezc run`, `ezc check`, `ezc repl`)
- `ezc-lsp` -- language server (stub)
- `ezc-web` -- WASM embedding (stub)

## Build & Test

```bash
cargo build                                    # build all crates
cargo test                                     # run all tests
cargo test -p ezc                              # test core crate only
cargo insta test                               # run snapshot tests
cargo insta review                             # review pending snapshots
cargo run -p ezc-cli -- run <file.ezc>         # run a program
cargo run -p ezc-cli -- check <file.ezc>       # parse-check only
cargo run -p ezc-cli -- repl                   # interactive TUI REPL
cargo run -p ezc-cli -- repl --no-tui          # plain reedline REPL
```

## Language Quick Reference

Values are pushed onto the stack. Operators pop arguments and push results.

```
3 4 +             # → 7
3 4 + 2 *         # → 14
(2 *) !           # execute a block
[1 2 3]           # create a list
"hello"           # push a string
42u8              # typed integer literal
3.14              # float literal (f64)
0xFF              # hex integer
1 2 ~             # swap → 2 1
5 @x $x $x +     # variables → 10
{ 10 @t $t 2 * }  # scoped bindings → 20
42 f32            # type constructor → 42.0f32
```

### Operators

Math: `+ - * / % ^`
Stack: `,` (dup), `;` (drop), `~` (swap), `_` (over)
Control: `!` (exec/splat/eval), `?` (cond drop), `??` (ternary), `&` (loop)
Compose: `|` (concat lists, strings, or blocks)
Higher-order: `&!` (map), `&?` (filter), `&/` (fold)
Comparison: `== != < > <= >=` (push 1 or 0)
I/O: `:` (write line), `.` (read line), `rl`/`wl`/`rb`/`wb`
Variables: `@name` (bind), `$name` (recall)
Scoping: `{...}` (local bindings)

### Type System

Numeric: `int` (BigInt), `u8`-`u256`, `i8`-`i256`, `f16`/`f32`/`f64`
Other: `str`, `bin`, `block`, `list`

Type names are constructors: `42 f32` converts int to f32.
Within-family arithmetic promotes (u8+u32→u32). Cross-family is an error.

Comments: `# rest of line`.

## Architecture

Three-phase pipeline: **Lex** → **Parse** → **Eval**

- `lexer.rs`: `&str → Vec<(Token, Span)>` — chumsky character-level tokenizer
- `parser.rs`: `Vec<(Token, Span)> → Vec<(Expr, Span)>` — chumsky token-level parser
- `eval.rs`: `Engine` — stack machine with scope chain and span-tagged values
- `number.rs`: `Number` enum (16 variants) + arithmetic with family promotion
- `types.rs`: `Value` enum, `EzStr`, `EzBin`, `Block`
- `intern.rs`: per-engine deduplication for str/bin/BigInt (Arc::ptr_eq equality)
- `error.rs`: error types + multi-label ariadne reports
- `lib.rs`: `ezc::run(src)`, `ezc::eval_line(engine, src)`

Key dependencies: `chumsky` (parsing), `ariadne` (error reports), `num-bigint` (integers),
`ethnum` (u256/i256), `half` (f16), `clap` (CLI), `reedline` (plain REPL),
`ratatui`/`crossterm` (TUI REPL), `tracing` (logging), `insta` (snapshot tests).

## Engine API

`Engine` is the core interpreter. It bundles:
- Stack: `Vec<Tagged>` — values tagged with source spans
- Environment: `Vec<HashMap<String, Value>>` — scope chain
- Interner: deduplicates str/bin/BigInt

```rust
let mut engine = Engine::new();
ezc::eval_line(&mut engine, "3 4 +")?;
// engine.stack() → [7]
```

## Testing

- Unit tests in each module (run with `cargo test`)
- Snapshot tests: `.ezc` files in `tests/ezc/<category>/`, `.snap` files colocated
- Error snapshots use `report_plain()` for readable ariadne output without ANSI
- Run `cargo insta test` to generate, `cargo insta test --accept` to accept
- All tests must pass before committing

## Code Conventions

- `thiserror` for error enums, `tracing` for debug output (never `println!` for debug)
- Strong types everywhere -- no stringly-typed code
- All immutable heap values (str, bin, BigInt) interned via `Arc` with pointer-equality fast path
- Stack entries carry source spans (`Tagged`) for rich error diagnostics
- Functional style: prefer immutable data, pure functions, pattern matching

## Flow System

Structured workflow for planning and tracking work. See `flow/README.md`.

Stages: `todo` → `work` → `done`
Each document: `flow/<stage>/NNN-slug.md`
