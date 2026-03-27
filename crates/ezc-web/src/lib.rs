//! `ezc-web` — WASM bindings for the ezc language.
//!
//! Exposes `ezc::run` and an incremental `EzcEngine` to JavaScript via
//! `wasm-bindgen`. Build with:
//!
//! ```bash
//! wasm-pack build crates/ezc-web --target web --out-dir ../../extras/playground/pkg
//! ```
//!
//! The generated package can be imported in the web playground as:
//!
//! ```js
//! import init, { EzcEngine, run } from "./pkg/ezc_web.js";
//! await init();
//! const result = run("3 4 +");
//! console.log(result); // { "ok": true, "stack": ["7"], "error": null }
//! ```

use wasm_bindgen::prelude::*;

// ── One-shot run ──────────────────────────────────────────────────────────

/// Run an EZC program and return a JSON result object.
///
/// On success: `{ "ok": true, "stack": ["7", "..."], "error": null }`
/// On error:   `{ "ok": false, "stack": [], "error": "..." }`
#[wasm_bindgen]
pub fn run(src: &str) -> JsValue {
    match ezc::run(src) {
        Ok(stack) => {
            let items: Vec<String> = stack.iter().map(|v| v.to_string()).collect();
            serde_wasm_bindgen_value(&serde_json::json!({
                "ok": true,
                "stack": items,
                "error": null,
            }))
        }
        Err(e) => serde_wasm_bindgen_value(&serde_json::json!({
            "ok": false,
            "stack": [],
            "error": e.to_string(),
        })),
    }
}

/// Parse an EZC program and return diagnostics as a JSON array.
///
/// Returns `[]` on success, or an array of `{ "message": "...", "start": N, "end": N }`.
#[wasm_bindgen]
pub fn check(src: &str) -> JsValue {
    match ezc::lex_and_parse(src) {
        Ok(_) => serde_wasm_bindgen_value(&serde_json::json!([])),
        Err(ezc::error::EzError::Parse(errs)) => {
            let diags: Vec<_> = errs
                .iter()
                .map(|e| {
                    serde_json::json!({
                        "message": e.message,
                        "start": e.span.start,
                        "end": e.span.end,
                    })
                })
                .collect();
            serde_wasm_bindgen_value(&serde_json::json!(diags))
        }
        Err(e) => serde_wasm_bindgen_value(&serde_json::json!([{
            "message": e.to_string(),
            "start": 0,
            "end": 0,
        }])),
    }
}

// ── Persistent engine ─────────────────────────────────────────────────────

/// A persistent EZC engine instance for interactive use (REPL, playground).
///
/// ```js
/// const engine = new EzcEngine();
/// engine.eval("5 @x");
/// const result = engine.eval("$x $x *");
/// // result.stack === ["25"]
/// ```
#[wasm_bindgen]
pub struct EzcEngine {
    engine: ezc::eval::Engine,
}

#[wasm_bindgen]
impl EzcEngine {
    /// Create a new engine with an empty stack and environment.
    #[wasm_bindgen(constructor)]
    pub fn new() -> Self {
        EzcEngine {
            engine: ezc::eval::Engine::new(),
        }
    }

    /// Evaluate a line of EZC source against the engine's current state.
    ///
    /// Returns `{ "ok": true, "stack": [...] }` or `{ "ok": false, "error": "..." }`.
    #[wasm_bindgen]
    pub fn eval(&mut self, src: &str) -> JsValue {
        match ezc::eval_line(&mut self.engine, src) {
            Ok(()) => {
                let items: Vec<String> =
                    self.engine.stack().iter().map(|v| v.to_string()).collect();
                serde_wasm_bindgen_value(&serde_json::json!({
                    "ok": true,
                    "stack": items,
                    "error": null,
                }))
            }
            Err(e) => serde_wasm_bindgen_value(&serde_json::json!({
                "ok": false,
                "stack": self.engine.stack().iter().map(|v| v.to_string()).collect::<Vec<_>>(),
                "error": e.to_string(),
            })),
        }
    }

    /// Return the current value stack as a JSON array of strings.
    #[wasm_bindgen(getter)]
    pub fn stack(&self) -> JsValue {
        let items: Vec<String> = self.engine.stack().iter().map(|v| v.to_string()).collect();
        serde_wasm_bindgen_value(&serde_json::json!(items))
    }

    /// Clear the stack and all variable bindings.
    #[wasm_bindgen]
    pub fn reset(&mut self) {
        self.engine.reset();
    }
}

impl Default for EzcEngine {
    fn default() -> Self {
        Self::new()
    }
}

// ── Helpers ───────────────────────────────────────────────────────────────

/// Serialize a serde_json::Value to a JsValue.
///
/// In a WASM context, `JsValue::from_serde` uses JSON.parse internally.
/// We serialize to a JSON string and let wasm-bindgen convert it.
fn serde_wasm_bindgen_value(v: &serde_json::Value) -> JsValue {
    // JsValue::from_serde is the old API; use serde-wasm-bindgen or
    // manual JSON string approach for compatibility.
    // The simplest portable approach that works with wasm-bindgen 0.2:
    match serde_json::to_string(v) {
        Ok(s) => JsValue::from_str(&s),
        Err(_) => JsValue::NULL,
    }
}
