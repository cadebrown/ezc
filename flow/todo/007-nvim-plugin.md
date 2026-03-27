# 007: Neovim Plugin

Status: todo
Created: 2026-03-26

## Context

Lua plugin for Neovim: ftdetect, ftplugin, tree-sitter queries, LSP config.

## Plan

Create `extras/nvim-ezc/` with:
1. `ftdetect/ezc.lua` — filetype detection
2. `ftplugin/ezc.lua` — commentstring, shiftwidth
3. `queries/ezc/highlights.scm`
4. `queries/ezc/folds.scm`
5. `lua/ezc/init.lua` — setup + LSP config
