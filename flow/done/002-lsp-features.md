# 002: Full LSP/DAP Feature Set
Status: done
Created: 2026-03-28
Completed: 2026-04-09

## Phase 1: Quick Wins

- [x] Folding ranges — fold `(...)`, `[...]`, `{...}` blocks
- [x] Document links — make `"std/math.ezc" import` clickable → opens the file
- [x] Signature help — show stack effects when cursor is on an operator
- [ ] ~~Status bar — "ezc: ready" with error count~~ (deferred — low value)

## Phase 2: High Impact

- [x] Code actions — "undefined variable: did you mean $x?", auto-import
- [x] Inlay hints — show stack state inline after each expression
- [x] Formatting — consistent spacing, indentation in blocks
- [x] CodeLens — "N references" above each @name definition
- [x] Workspace symbols — Ctrl+T search across all .ezc files

## Phase 3: Advanced

- [ ] ~~Call hierarchy — incoming/outgoing calls for functions~~ (deferred)
- [x] DAP evaluate expression — REPL in debug console
- [x] DAP conditional breakpoints — "break when stack top > 100"
- [x] Selection ranges — smart expand: token → expression → block
