# 007: Editor Plugins — Neovim, Zed, Helix

Status: done
Created: 2026-03-27
Updated: 2026-03-27

## Context

Editor plugins for Neovim (Lua), Zed (extension.toml), and Helix (languages.toml).
All three wire up the tree-sitter grammar, LSP (ezc lsp), and DAP (ezc debug).

## Files

- `extras/nvim-ezc/lua/ezc/init.lua` — Neovim plugin (filetype detection, LSP
  via `vim.lsp.start`, nvim-dap config, tree-sitter parser registration)
- `extras/nvim-ezc/ftdetect/ezc.vim` — Vimscript filetype detection
- `extras/nvim-ezc/ftplugin/ezc.vim` — Buffer-local settings (commentstring, indent)
- `extras/nvim-ezc/queries/ezc/highlights.scm` — Neovim tree-sitter highlights
- `extras/nvim-ezc/queries/ezc/locals.scm` — Variable tracking (go-to-def, references)
- `extras/nvim-ezc/queries/ezc/folds.scm` — Fold regions
- `extras/zed-ezc/extension.toml` — Zed extension manifest
- `extras/zed-ezc/languages/ezc/config.toml` — Language config (LSP + DAP debugger)
- `extras/zed-ezc/languages/ezc/highlights.scm` — Zed highlight queries
- `extras/zed-ezc/languages/ezc/brackets.scm` — Bracket matching
- `extras/helix-ezc/languages.toml` — Helix language + grammar + LSP + DAP config
- `extras/helix-ezc/queries/highlights.scm` — Helix highlight queries

## Installation

**Neovim (lazy.nvim):**
```lua
{ dir = "path/to/ezc/extras/nvim-ezc", ft = "ezc",
  config = function() require("ezc").setup() end }
```

**Helix:** Merge `languages.toml` into `~/.config/helix/languages.toml`,
copy tree-sitter sources, run `hx --grammar build`.

**Zed:** Copy `extras/zed-ezc/` to `~/.config/zed/extensions/ezc/`.
