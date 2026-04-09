# Debugger and stepper integration (EZC core)

This document explains **why** breakpoints and stepping sometimes miss inner code, and **how** the interpreter and stepper stay aligned.

## The invariant

The debug adapter drives execution through:

```text
Stepper::step_over → Engine::eval_one(&Spanned<Expr>) → eval_expr(...)
```

Breakpoints are taken when `Stepper::breakpoint_at_current()` sees a **1-based source line** (and optional **column** and **source path**) that matches the **next** expression’s span (`StepFrame::current_span` → that frame’s `LineIndex`).

**Any path that runs a sub-program with `Engine::eval(&[Spanned<Expr>])` instead of stepping each expression hides inner PCs from the stepper.** The stepper therefore **intercepts** those forms before `eval_one` runs them as a whole.

## Current coverage (stepper intercepts)

| Feature | Stepper mechanism |
|--------|-------------------|
| `&` loop | `LoopDriver` + `pop_loop_blocks` |
| `each` / `map` / `fil` / `red` / `&!` / `&?` / `&/` | Per-iteration frames + drivers |
| `{…}` scope | Child frame + `scopes_to_pop` / `push_scope` |
| `[ … ]` list literal | `ListBuildDriver` + `finalize_list_literal` |
| `import` | `prepare_import_step` + module frame with module `LineIndex` |
| Bare `name` when bound to a block | `lookup_autoexec_block` + child frame |
| `!` + block | Child frame (same as prior `step_in` behavior; `step_over` enters too) |
| `!` + string | Parsed snippet AST + snippet `LineIndex` |
| `!` + list (splat) | `SplatDriver` one push per step |
| `?` truthy + block | Pops operands, child frame for block body |
| `??` + block branch | Pops operands, child frame when chosen value is a block |

**Still one atomic `eval_one` step:** pushing a **block literal** `( … )` (no `!`) — inner expressions are not evaluated until the block runs.

## Breakpoints

- `Breakpoint` is a **vector** (order preserved; first match wins).
- **`column: Option<u32>`** — 1-based column on the line; `None` matches any column on that line.
- **`source_path: Option<String>`** — when `Some`, must match the current frame’s `source_path` via `ezc::debug_source_paths_equivalent` (file URLs, trim, optional `canonicalize` — same rules as the DAP adapter). `None` matches every frame (typical for unit tests).

## Files

| File | Role |
|------|------|
| `crates/ezc/src/debug_source_path.rs` | `debug_source_paths_equivalent` — shared by stepper + DAP |
| `crates/ezc/src/eval.rs` | `prepare_import_step`, list/cond/ternary/snippet helpers, `ident_skips_lookup`, `lookup_autoexec_block` |
| `crates/ezc/src/stepper.rs` | Drivers, per-frame `LineIndex`, `try_begin_*`, `advance_composite_drivers` |
| `crates/ezc-dap/src/server.rs` | Maps DAP `SourceBreakpoint.column` and `source.path` into `Breakpoint` |

## Verification

```bash
cargo test -p ezc stepper::
cargo test -p ezc-dap
```

## Related

- DAP server: `crates/ezc-dap/src/server.rs`
- VS Code extension: `editors/vscode-ezc/`
