# 011: Web Playground

Status: todo
Created: 2026-03-26

## Context

Interactive web playground — try EZC in the browser, share programs via URL.

## Plan

1. Create `extras/playground/` with HTML/CSS/JS
2. CodeMirror 6 editor with EZC language support
3. Load WASM module, Run button executes code
4. Share via URL hash (base64 encoded)
5. Example programs dropdown
