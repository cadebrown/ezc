# EZC Playground

Browser-based REPL for EZC powered by the WASM-compiled core.

## Build

```bash
# Install wasm-pack if needed
cargo binstall wasm-pack

# Build the WASM module (output goes to extras/playground/pkg/)
wasm-pack build crates/ezc-web --target web --out-dir ../../extras/playground/pkg

# Serve (any static file server works)
npx serve extras/playground
# or
python3 -m http.server -d extras/playground
```

## Features

- **Run** — execute the program and display the final value stack
- **Trace** — step line-by-line, showing the stack after each expression
- **Share** — encode the program in the URL hash for sharing

## Notes

- The playground uses the same `ezc::run` entry point as the CLI binary.
- `EzcEngine` exposes a persistent engine for incremental evaluation (used by the Trace view).
- CORS is required to load the WASM module; use a local server rather than opening `index.html` directly.
