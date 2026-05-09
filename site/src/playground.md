# Playground

Type ezc, hit **Run** (or `Ctrl+Enter`). Everything runs in your browser
via WebAssembly — no installation, no network calls.

<div id="ezc-full-playground"></div>

## Tips

- **Run** executes the program and shows the resulting stack.
- **Trace** evaluates each line independently and shows the stack after
  each step — handy for understanding how the stack evolves.
- **Share** copies a permalink that includes your code in the URL hash.
- **Clear** wipes the editor and output panes.

The full WASM API surface is in [`crates/ezc-web`](https://github.com/cadebrown/ezc/tree/main/crates/ezc-web)
if you want to embed ezc in your own page.
