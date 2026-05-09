# 012: Documentation Site

Status: done
Created: 2026-03-26
Completed: 2026-04-09

## Context

Static documentation site. Deploy to any static host.

## Outcome

mdbook-based site at `site/` with:

- Landing page + getting started
- Six-chapter tutorial (basics, variables, functions, lists, types, I/O)
- Reference (operators, builtins, type system, stdlib)
- Editor setup (VS Code, Neovim, Zed, Helix)
- Examples gallery
- Full-page playground (WASM)

Custom theme in `site/theme/`:
- `playground.js` — wires runnable Run buttons onto every `ezc` code block
- `playground.css` — styling

Build: `site/build.sh` runs `wasm-pack` + `mdbook build` and copies the
WASM bundle into the book output. Deploy `site/book/` anywhere.

No domain hardcoded. Works on file://, GitHub Pages, Cloudflare Pages,
or any static host.
