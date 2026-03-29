# EZC for VS Code / Cursor

Language support for [ezc](https://github.com/cadebrown/ezc) — a stack-based, reverse-polish-notation programming language.

## Features

- **Syntax highlighting** — numbers, strings, operators, variables, type constructors
- **Semantic tokens** — LSP-powered highlighting that distinguishes user functions from builtins
- **Hover documentation** — hover over any operator or builtin for docs and stack effects
- **Completions** — operators, builtins, type constructors, and variables in scope
- **Go-to-definition** — Cmd+Click on `$name` or a bare word to jump to its `@name` definition
- **Find all references** — right-click → Find All References on any variable
- **Rename** — F2 to rename a variable across `@name`, `$name`, and bare-word uses
- **Document symbols** — outline view showing all function and variable definitions
- **Diagnostics** — parse errors shown inline with source annotations
- **Debugging** — full DAP support: breakpoints, step in/out/over, stack and variable inspection
- **Snippets** — common patterns (blocks, conditionals, loops, imports)

## Requirements

**Easiest (one binary on PATH):** the CLI includes LSP and DAP as subcommands.

```bash
cargo install --path crates/ezc-cli
# Provides `ezc`, `ezc lsp`, and `ezc dap` (add ~/.cargo/bin to PATH if needed)
```

**Optional standalone binaries** (same as `ezc lsp` / `ezc dap`):

```bash
cargo install --path crates/ezc-lsp
cargo install --path crates/ezc-dap
```

**From a repo clone** (no install):

```bash
cargo build -p ezc-cli -p ezc-lsp -p ezc-dap
# target/debug/ezc, ezc-lsp, ezc-dap
```

## Usage

1. Open any `.ezc` file — the extension activates automatically
2. The LSP server starts and provides hover, completions, diagnostics, etc.
3. Press **F5** to debug the current file (sets breakpoints, steps, inspects stack)
4. Use **Cmd+Click** on variables to jump to definitions
5. Use **F2** to rename variables across the file

## Settings

| Setting | Default | Description |
|---------|---------|-------------|
| `ezc.lsp.path` | *(empty)* | Absolute path to `ezc-lsp`; empty = auto |
| `ezc.dap.path` | *(empty)* | Absolute path to `ezc-dap`; empty = auto |
| `ezc.trace.dap` | `true` | Log DAP to **Output → EZC**; sets `RUST_LOG` for the adapter |
| `ezc.trace.server` | `off` | `messages` / `verbose` → **Output → EZC LSP** |

**Resolution order** (when path settings are empty):

1. `../../target/debug/ezc-lsp` (or `ezc-dap`) next to this extension — works when the repo is laid out as `ezc/editors/vscode-ezc/`.
2. `<workspaceFolder>/target/debug/…` — useful if you opened the **repo root** (`ezc/`) as the workspace.
3. Run `ezc lsp` / `ezc dap` via PATH (needs `ezc` on the **editor’s** PATH, not only your terminal — see Development).

Override with absolute paths if the host does not see `ezc` on PATH.

## Development

### 1. Open the extension folder as the workspace

Use **File → Open Folder** on `editors/vscode-ezc` (not only the `.ezc` file).  
`F5` / **Run → Start Debugging** uses `.vscode/launch.json`, which sets `--extensionDevelopmentPath` to that folder.

### 2. Build Rust, then TypeScript

From the repo root:

```bash
cargo build -p ezc-cli -p ezc-lsp -p ezc-dap
```

In the editor, the default build task (**Terminal → Run Build Task**) runs, in order:

1. `cargo build -p ezc-cli -p ezc-lsp -p ezc-dap` with `cwd` = repo root (`../..` from this folder)
2. `npm run compile`

The Extension launch configs use that compound task as **preLaunchTask**, so each F5 rebuilds the toolchain and recompiles the extension.

### 3. PATH and the Extension Host

Cursor/VS Code often **do not** load your shell profile, so `ezc` may work in Terminal but **not** inside the Extension Host. In that case:

- Rely on **(1)** `../../target/debug/…` after `cargo build` (typical monorepo layout), or  
- Set **`ezc.lsp.path`** / **`ezc.dap.path`** to absolute binaries (e.g. `/path/to/ezc/target/debug/ezc-lsp`), or  
- Launch the GUI from a shell where `PATH` includes `~/.cargo/bin`, or set the same in your OS environment for GUI apps.

Use **Output → EZC** (command **EZC: Show Output Log**) to see which command line the extension actually spawned.

### 4. Debug a sample `.ezc` file

Launch configs open `../test-workspace` or `../../demos`. Set breakpoints, use the **EZC** debug configuration, and confirm the EZC output channel shows DAP traffic if `ezc.trace.dap` is on.

### Project structure

```
editors/vscode-ezc/
  src/extension.ts       # extension entry point (LSP + DAP client)
  syntaxes/              # TextMate grammar for basic highlighting
  snippets/              # code snippets
  .vscode/launch.json    # F5 launch configs for development
  .vscode/tasks.json     # cargo + npm preLaunch build chain
```

### Testing

The `editors/test-workspace/` directory contains test files:

| File | What to test |
|------|-------------|
| `scratch.ezc` | All LSP features (hover, go-to-def, references, rename) |
| `debug_me.ezc` | Debugger (breakpoints, stepping, variables) |
| `errors.ezc` | Diagnostics (uncomment lines to trigger errors) |

## Language overview

```ezc
# Stack-based RPN — values pushed, operators pop and push results
3 4 + 2 *                    # → 14

# Variables and functions
(, *) @square                # define
5 square                     # call (bare word auto-executes blocks)

# Conditionals
x 0 > ("positive") ?        # conditional execute
x 0 > ("pos") ("neg") ??    # if-else

# Lists and higher-order
[1 2 3] (2 *) map            # → [2 4 6]
[1 2 3 4] (even) fil         # → [2 4]
[1 2 3] 0 (+) red            # → 6
1 101 range (fizzbuzz) each  # iterate

# I/O
"hello" :                    # print with newline
. @input                     # read line from stdin

# Types
42u8 3.14f32 "hello" 0xFF
42 f64                       # type constructor
```

See [docs/language.md](../../docs/language.md) for the full reference.
