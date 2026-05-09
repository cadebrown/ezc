# Editor setup

ezc ships with first-class plugins for four editors. All connect to the
same `ezc-lsp` (language server) and `ezc-dap` (debugger) binaries, so
you get the same features everywhere: diagnostics, hover, completions,
go-to-definition, rename, references, code lens, inlay hints, code
actions, formatting, folding, document symbols, signature help, and
full step-debugging.

Build the binaries first:

```bash
cargo install --path crates/ezc-cli
cargo install --path crates/ezc-lsp
cargo install --path crates/ezc-dap
```

## VS Code

The extension lives at `editors/vscode-ezc/`. It includes a TextMate
grammar, snippets, and configuration for both the LSP client and a DAP
launch profile.

```bash
cd editors/vscode-ezc
npm install
npm run compile
# then: F5 in VS Code to launch a development host
# or: vsce package and install the .vsix
```

Configurable settings:

- `ezc.lsp.path`, `ezc.dap.path` — override binary lookup
- `ezc.inlayHints.stackState` — show post-line stack state inline
- `ezc.inlayHints.references` — show "N references" markers

## Neovim

Plugin at `editors/neovim-ezc/`. Install with lazy.nvim:

```lua
{
  dir = "/path/to/ezc/editors/neovim-ezc",
  ft = "ezc",
  dependencies = { "neovim/nvim-lspconfig", "mfussenegger/nvim-dap" },
  config = function()
    require("ezc").setup({})
    require("ezc").set_dap_keymaps()  -- F5/F10/F11/F12 for debugging
  end,
}
```

Or copy the plugin to `~/.local/share/nvim/site/pack/ezc/start/` and
call `require("ezc").setup()` in your config.

## Zed

Extension at `editors/zed-ezc/`. Symlink it into your Zed extensions
directory:

```bash
ln -s "$PWD/editors/zed-ezc" ~/.config/zed/extensions/ezc
```

Then restart Zed. Both LSP and DAP work natively.

## Helix

Config at `editors/helix-ezc/`. Merge `languages.toml` into your Helix
config:

```bash
cat editors/helix-ezc/languages.toml >> ~/.config/helix/languages.toml
```

For tree-sitter highlighting, copy the grammar:

```bash
cp -r editors/tree-sitter-ezc ~/.config/helix/runtime/grammars/sources/ezc/
hx --grammar build
```

## Tree-sitter grammar

Standalone tree-sitter grammar at `editors/tree-sitter-ezc/`. Used by
Neovim, Zed, and Helix; can also be embedded in any other tree-sitter
host.
