# 010: WASM Core

Status: todo
Created: 2026-03-26

## Context

Compile `ezc` to WebAssembly. Foundation for the web playground.

## Plan

1. Add wasm-bindgen to ezc-web/Cargo.toml
2. Expose `run(src) -> JsValue` and `tokenize(src) -> JsValue`
3. Build with `wasm-pack build --target web`
