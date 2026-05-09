# 010: WASM Core

Status: done
Created: 2026-03-26
Completed: 2026-03-30

## Context

Compile `ezc` to WebAssembly. Foundation for the web playground.

## Outcome

`crates/ezc-web/` exposes:
- `run(src)` — one-shot execution returning `{ ok, stack, error }`
- `check(src)` — parse-only validation returning a diagnostic list
- `EzcEngine` class — persistent engine for REPL-style use (`eval()`, `stack`, `reset()`)

Build: `wasm-pack build crates/ezc-web --target web --out-dir ../../site/public/pkg --release`
