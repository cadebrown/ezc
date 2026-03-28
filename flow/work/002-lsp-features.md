# 002: Full LSP/DAP Feature Set
Status: work
Created: 2026-03-28

## Phase 1: Quick Wins

- [ ] Folding ranges — fold `(...)`, `[...]`, `{...}` blocks
- [ ] Document links — make `"std/math.ezc" import` clickable → opens the file
- [ ] Signature help — show stack effects when cursor is on an operator
- [ ] Status bar — "ezc: ready" with error count

## Phase 2: High Impact

- [ ] Code actions — "undefined variable: did you mean $x?", auto-import
- [ ] Inlay hints — show stack state inline after each expression
- [ ] Formatting — consistent spacing, indentation in blocks
- [ ] CodeLens — "N references" above each @name definition
- [ ] Workspace symbols — Ctrl+T search across all .ezc files

## Phase 3: Advanced

- [ ] Call hierarchy — incoming/outgoing calls for functions
- [ ] DAP evaluate expression — REPL in debug console
- [ ] DAP conditional breakpoints — "break when stack top > 100"
- [ ] Selection ranges — smart expand: token → expression → block
