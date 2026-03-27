# 009: WASM Core & Web Playground

Status: done
Created: 2026-03-27
Updated: 2026-03-27

## Context

WASM bindings for `ezc-web` and a browser-based playground at
`extras/playground/index.html`.

## Files

- `crates/ezc-web/src/lib.rs` — wasm-bindgen bindings:
  - `run(src) → JSON` — one-shot run, returns `{ok, stack, error}`
  - `check(src) → JSON` — parse-only, returns array of diagnostics
  - `EzcEngine` class — persistent engine for incremental evaluation
- `extras/playground/index.html` — single-file playground (no bundler needed)
- `extras/playground/README.md` — build instructions

## Build

```bash
wasm-pack build crates/ezc-web --target web --out-dir ../../extras/playground/pkg
npx serve extras/playground
```

## Features

- Run — execute program, show final value stack
- Trace — evaluate line-by-line, show stack state after each line
- Share — encode program in URL hash for sharing
- Dark theme, keyboard shortcut (Ctrl+Enter), tab indentation

## Return format

```json
{ "ok": true,  "stack": ["7"],  "error": null }
{ "ok": false, "stack": [],     "error": "division by zero" }
```

Diagnostics from `check`:
```json
[{ "message": "...", "start": 4, "end": 7 }]
```
