# ezcweb

Standalone Dioxus web app for EZC docs + REPL + learning book.

Requires `dioxus-cli 0.7.2` to match this crate's pinned Dioxus version.

Routes:

- `/` landing page + runnable samples
- `/repl` full terminal REPL (`xterm.js`)
- `/docs` language docs
- `/book` chapter index for guided learning
- `/book/:slug` chapter pages with prev/next navigation

Offline/static deploy notes:

- `xterm.js` is vendored locally under `assets/vendor/xterm/` (no CDN dependency).
- Release output is fully static at `../target/dx/ezcweb/release/web/public/`.

## Run

From repo root:

```bash
dx serve -p ezcweb --web
```

From this directory:

```bash
dx serve
```

Use a different port if needed:

```bash
dx serve --port 4310
```

## Build Static

```bash
dx build -p ezcweb --web --release
```

Output:

`../target/dx/ezcweb/release/web/public/` (when run from `ezcweb/`)

## Cloudflare Pages

Infrastructure for Cloudflare Pages lives in:

- `../infra/cloudflare/main.tf`
- `../infra/cloudflare/pages_build.sh`

The default build config in infra is set up for this Rust/Dioxus workspace and publishes `ezcweb` to `ezc.cade.io`.
