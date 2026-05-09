# 016: CI / Release Automation

Status: done
Created: 2026-03-26
Completed: 2026-04-09

## Outcome

Three GitHub Actions workflows under `.github/workflows/`:

- **ci.yml** — runs on every PR + push to main:
  - cargo check + test on Linux + macOS
  - clippy --all-targets -- -D warnings + fmt --check
  - end-to-end site build (catches WASM/mdbook regressions)
- **site.yml** — auto-deploys docs site to GitHub Pages on push to main
  when `site/`, `crates/ezc-web/`, `crates/ezc/`, or `std/` changes
- **release.yml** — on `v*` tag: builds cross-platform binaries (Linux
  x64+arm64, macOS Intel+Apple Silicon, Windows x64), packages them
  with std/ + LICENSE, attaches to a GitHub Release with auto-generated
  notes

Pre-existing clippy warnings (collapsible_match in tui/app.rs,
redundant_closure in lsp_tests) fixed so the first CI run will be
green. All workflows lint cleanly with actionlint.

Not done (deferred): VSIX packaging, Homebrew formula,
cargo-binstall metadata. Add later if there's demand.
