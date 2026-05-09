# 016: Deploy + Release Automation

Status: done
Created: 2026-03-26
Completed: 2026-04-09

## Outcome

Cloudflare Pages handles CI and deployment via its built-in GitHub
integration. **No GitHub Actions** in this repo.

- `infra/cloudflare/main.tf` — OpenTofu config for the Cloudflare Pages
  project + custom domain (`ezc.cade.io`) + DNS CNAME on the existing
  `cade.io` zone.
- `site/cf-build.sh` — Cloudflare Pages build entry point. Installs
  pinned `wasm-pack` and `mdbook` release binaries, then delegates to
  `site/build.sh` which assembles `site/book/`.
- `infra/README.md` — setup instructions (`tofu init` / `plan` / `apply`).

Pushes to `main` automatically trigger a Cloudflare Pages build via the
GitHub source binding. Output is published to `https://ezc.cade.io`.

Pre-existing clippy warnings (collapsible_match in tui/app.rs,
redundant_closure in lsp_tests) fixed in the same pass that introduced
the deploy infra, so local `cargo clippy --workspace --all-targets
--all-features -- -D warnings` is clean.

Not done (deferred): cross-platform release binaries on `vX.Y.Z` tags,
VSIX packaging, Homebrew formula. Add later if there's demand.
