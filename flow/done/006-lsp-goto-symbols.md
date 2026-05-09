# 006: LSP Phase C — Go-to-Definition + Symbols

Status: done
Created: 2026-03-26
Completed: 2026-04-09

## Context

Navigate from `$name` to its `@name` binding. Document outline of all bindings.

## Plan

1. Go-to-definition: `$name` → `@name` (walk AST respecting `{...}` scopes)
2. Find references: all uses of a variable name
3. Document symbols: list all top-level `@name` bindings
