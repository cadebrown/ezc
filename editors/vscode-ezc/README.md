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

Install the ezc toolchain:

```bash
cargo install --path crates/ezc-cli    # ezc (runtime)
cargo install --path crates/ezc-lsp    # language server
cargo install --path crates/ezc-dap    # debug adapter
```

Or build from source:

```bash
cargo build
# Binaries are in target/debug/ezc, target/debug/ezc-lsp, target/debug/ezc-dap
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
| `ezc.lsp.path` | `"ezc-lsp"` | Path to the LSP binary |
| `ezc.dap.path` | `"ezc-dap"` | Path to the DAP binary |

During development, the extension auto-detects `target/debug/ezc-lsp` and `target/debug/ezc-dap` relative to the extension directory.

## Development

```bash
# Build the language toolchain
cargo build

# Build the extension
cd editors/vscode-ezc
npm install
npm run compile

# Launch in development mode (F5 in VS Code/Cursor)
# Opens editors/test-workspace/ with the extension loaded
# LSP/DAP automatically use target/debug/ builds
```

### Project structure

```
editors/vscode-ezc/
  src/extension.ts       # extension entry point (LSP + DAP client)
  syntaxes/              # TextMate grammar for basic highlighting
  snippets/              # code snippets
  .vscode/launch.json    # F5 launch configs for development
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
