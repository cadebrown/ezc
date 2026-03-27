# 003: VS Code Extension (with LSP client)

Status: done
Created: 2026-03-26
Updated: 2026-03-26

## Context

Full VS Code extension: TextMate grammar, language config, snippets, and
TypeScript LSP client that spawns `ezc lsp` on activation.

## Verification

- `npm run compile` produces `out/extension.js` with no errors
- F5 in `extras/vscode-ezc/` launches Extension Development Host
- `.ezc` files get syntax highlighting, bracket matching, comment toggling
- Snippets expand (loop, map, filter, fn, fold, etc.)
- LSP client connects when `ezc` binary is in PATH

## Lessons Learned

Used `vscode-languageclient` 9.x for the LSP client. Pure stdio transport to
`ezc lsp`. Server path configurable via `ezc.server.path` setting.
