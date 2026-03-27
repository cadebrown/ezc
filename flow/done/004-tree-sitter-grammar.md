# 004: Tree-sitter Grammar

Status: done
Created: 2026-03-26
Updated: 2026-03-26

## Context

Incremental, error-recovering parser for EZC. Powers Neovim, Helix, Zed,
Emacs 29+ native tree-sitter mode.

## Verification

- `npx tree-sitter generate && npx tree-sitter test` — 28/28 tests pass
- Correctly parses all real `.ezc` test files
- Highlight, fold, indent queries all present

## Lessons Learned

Had to handle operator ambiguity carefully (e.g. `&!` before `&`, `??` before `?`).
The `_$` parameter is used for leaf rules to suppress warnings about unused params.
