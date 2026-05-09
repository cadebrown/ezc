# Infrastructure

OpenTofu manages the Cloudflare Pages project + DNS for `ezc.cade.io`.

## Scope

- Cloudflare Pages project (`ezc-cade-io`)
- `ezc` subdomain CNAME on the existing `cade.io` zone
- Pages → GitHub source binding (Cloudflare auto-builds on push to `main`)

This repo has **no GitHub Actions**. Cloudflare Pages handles CI and
deployment via its built-in GitHub integration.

## Prerequisites

- OpenTofu installed (`tofu`)
- Cloudflare API token with these scopes:
  - Account: Cloudflare Pages — Edit
  - Zone: Zone — Read
  - Zone: DNS — Edit

## Setup

```sh
cd infra/cloudflare
cp terraform.tfvars.example terraform.tfvars
```

Edit `terraform.tfvars`:

- `account_id  = "YOUR_CLOUDFLARE_ACCOUNT_ID"`
- `github_owner = "YOUR_GITHUB_USER_OR_ORG"`

Then:

```sh
export CLOUDFLARE_API_TOKEN="..."
tofu init
tofu plan
tofu apply
```

## What this creates

- Cloudflare Pages project `ezc-cade-io` wired to the GitHub repo
- `ezc.cade.io` custom domain bound to the project
- CNAME record `ezc` → `<project>.pages.dev` on the `cade.io` zone

## Build

Cloudflare Pages runs `bash site/cf-build.sh` on every push to `main`.
That script installs pinned versions of `wasm-pack` and `mdbook` from
release binaries, then delegates to `site/build.sh` which builds the
WASM bundle and the mdbook output. Output is published from `site/book`.

## Files

- `infra/cloudflare/main.tf` — single-file OpenTofu config
- `infra/cloudflare/terraform.tfvars.example` — copy to `terraform.tfvars`
- `infra/cloudflare/.gitignore` — ignores tfstate + your `tfvars`
