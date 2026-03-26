# CLAUDE.md -- ezc language project

## Project Overview

ezc is a stack-based, reverse-polish-notation programming language with a mathematical focus.
Big integers are the default numeric type. Code is a flat sequence of values and operators.

## Project Structure

Rust workspace with 4 crates under `crates/`:

- `ezc` -- core language (types, lexer, parser, evaluator)
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
cargo run -p ezc-cli -- repl                   # interactive REPL
```

## Language Quick Reference

Values are pushed onto the stack. Operators pop arguments and push results.

```
3 4 +           # → 7
3 4 + 2 *       # → 14
(2 *) !         # execute a block
[1 2 3]         # create a list
1 2 ~           # swap → 2 1
1 2 _           # over → 1 2 1
```

Operators: `+ - * / % ^` (math), `!` (execute), `?` (conditional), `??` (ternary),
`|` (compose), `&` (loop), `&!` (map), `&?` (filter),
`==` `!=` `<` `>` `<=` `>=` (comparison),
`:` (dup), `~` (swap), `_` (over), `$` `@` (reserved).

Comments: `# rest of line`.

## Architecture

Three-phase pipeline: **Lex** -> **Parse** -> **Eval**

- `lexer.rs`: `&str -> Vec<(Token, Span)>` — chumsky character-level tokenizer
- `parser.rs`: `Vec<(Token, Span)> -> Vec<(Expr, Span)>` — chumsky token-level parser
- `eval.rs`: stack machine evaluator — walks AST linearly
- `error.rs`: error types + ariadne pretty-printing
- `lib.rs`: `ezc::run(src) -> Result<Vec<Value>, EzError>`

Key dependencies: `chumsky` (parsing), `ariadne` (error reports), `num-bigint` (integers),
`clap` (CLI), `tracing` (logging), `insta` (snapshot tests).

## Testing

- Unit tests in each module (run with `cargo test`)
- Snapshot tests: add `.ezc` files to `tests/ezc/<category>/`, output `.snap` files colocated next to them
- Run `cargo insta test` to generate new snapshots, then accept them
- All tests must pass before committing

## Code Conventions

- `thiserror` for error enums, `tracing` for debug output (never `println!` for debug)
- Strong types everywhere -- no stringly-typed code
- Functional style: prefer immutable data, pure functions, pattern matching
- Document non-obvious design decisions with comments

## Flow System

Structured workflow for planning and tracking work. See `flow/README.md`.

Stages: `prop` -> `todo` -> `plan` -> `work` -> `done`

### Before starting work
1. Check `flow/work/` for in-progress items
2. Check `flow/prop/` and `flow/todo/` for existing related proposals
3. If no proposal exists, create one in `flow/prop/`
4. Follow stage transitions -- don't skip stages

### Document format
Each document: `flow/<stage>/NNN-slug.md`

Required header:
```
# NNN: Title
Status: prop | todo | plan | work | done
Created: YYYY-MM-DD
Updated: YYYY-MM-DD
```

### Adding a new feature
1. Write proposal in `flow/prop/NNN-slug.md`
2. Once accepted, move to `flow/todo/` and update status
3. Write detailed plan, move to `flow/plan/`
4. Implement, move to `flow/work/`, update Status Log
5. Complete, move to `flow/done/`, add Lessons Learned
6. Commit flow changes alongside code changes
