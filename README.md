# EZC

EZC is a concatenative, postfix, stack-based esolang implemented in Rust.

## Source Layout

- `src/ezclang/`: language frontend (`tokenizer`, `parser`)
- `src/ezcbc/`: EZC bytecode compiler/ISA
- `src/ezcvm/`: EZC virtual machine runtime
- `ezcweb/`: standalone Dioxus WASM docs/playground crate
- `docs/`: EZC language source of truth
- `test/`: EZC source-code fixtures used by tests

## CLI

```bash
cargo run -- run demo/square.ezc
cargo run -- run demo/hello.ezc
cargo run -- run demo/fib.ezc
cargo run -- run demo/gcd.ezc
cargo run -- run demo/factorial.ezc
cargo run -- run demo/powers_of_two.ezc
cargo run -- run demo/triangular.ezc
cargo run -- disasm demo/loop_countdown.ezc
cargo run -- check demo/substack.ezc
cargo run -- eval "(5 dup * prt)!"
cargo run -- repl
```

REPL controls:

- `:help` show commands
- `:clear` clear transcript
- `:quit` exit the session

Verbose intermediate stages:

```bash
cargo run -- --verbose run demo/loop_countdown.ezc
```

## Development

```bash
cargo test
cargo check --workspace
cargo check -p ezcweb
```

## Deployment (Cloudflare Pages)

Deployment infra is managed in `infra/` using OpenTofu/Terraform.

- Cloudflare Pages project is configured for this repo.
- Custom domain target is `ezc.cade.io`.

Quick start:

```bash
cd infra/cloudflare
cp terraform.tfvars.example terraform.tfvars
```

Set `account_id`, `github_owner`, and export your API token:

```bash
export CLOUDFLARE_API_TOKEN="..."
tofu init
tofu plan
tofu apply
```

This repo intentionally does not use GitHub Actions deployment.
Cloudflare build tooling bootstrap uses `infra/cloudflare/pages_build.sh` with
`cargo-binstall` fast-path and `cargo install` fallback.

## Webapp

Run it from the web crate:

```bash
cd ezcweb
dx serve
```

Or from repo root:

```bash
dx serve -p ezcweb --web
```

If you want a different port (recommended in this repo to avoid 8080 collisions):

```bash
cd ezcweb
dx serve --port 4310
```

The webapp is route-based:

- `/`: landing page with high-level overview and runnable samples
- `/repl`: full-screen `xterm.js` terminal, persistent VM stack, command history
- `/docs`: `docs/language.md` rendered with runnable snippets
- `/book`: chapter index for the learning walkthrough
- `/book/:slug`: chapter pages with previous/next navigation

The web bundle is self-contained for static hosting: terminal assets are vendored (no CDN runtime fetches).

### Setup

```bash
rustup target add wasm32-unknown-unknown
cargo install dioxus-cli --version 0.7.2 --locked
cargo install wasm-bindgen-cli
```

Build static output:

```bash
cd ezcweb
dx build --release
python3 -m http.server --directory ../target/dx/ezcweb/release/web/public 8080
```

Vendored web dependencies:

- `ezcweb/assets/vendor/xterm/xterm.js`
- `ezcweb/assets/vendor/xterm/xterm.css`
