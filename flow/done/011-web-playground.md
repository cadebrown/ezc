# 011: Web Playground

Status: done
Created: 2026-03-26
Completed: 2026-04-09

## Context

Interactive web playground — try EZC in the browser, share programs via URL.

## Outcome

Two complementary playgrounds:

1. **Standalone playground** at `editors/playground-ezc/` — single-file
   HTML for embedding or local use. Run / Trace / Share + dark theme.

2. **Embedded playground** at `site/src/playground.md` — full UI mounted
   inside the docs site (`site/theme/playground.js`). Same WASM
   backend, with stack/output/trace panes and base64 URL sharing.

Plus: every fenced ```ezc code block on the docs site gets an inline
**Run ▶** button (`setupInlineRunners` in `playground.js`), so tutorials
and reference pages are interactive.
