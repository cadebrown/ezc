# 005: LSP — Diagnostics, Hover, Completion

Status: done
Created: 2026-03-26
Updated: 2026-03-26

## Context

Full LSP implementation in `crates/ezc-lsp` using `tower-lsp`. Implements
diagnostics (parse errors), hover documentation for all operators and type
constructors, and completion (operators, types, variables).

## Plan

1. Add tower-lsp, tokio, serde_json to workspace Cargo.toml
2. Implement Backend struct with document storage
3. Diagnostics: lex+parse on every change, publish errors as LSP diagnostics
4. Hover: re-lex to find token at cursor, return markdown docs
5. Completion: static operator/type list + dynamic variable names from @bindings
6. Add `ezc lsp` subcommand to CLI
7. Add TypeScript client to VS Code extension

## Verification

- `cargo build --all` passes
- `ezc lsp` starts and handles LSP initialize
- VS Code shows red squiggles on parse errors
- Hovering over `+` shows stack effect and description
- Completion offers all operators and type constructors

## Lessons Learned

Used `enable_all()` instead of `enable_io()` on the tokio runtime builder.
The LSP server runs as a standalone binary via `ezc lsp` (stdio transport).
Variable names extracted by scanning @name bindings with the lexer.
