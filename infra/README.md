# Infrastructure

This folder contains OpenTofu/Terraform config for deploying EZC web to Cloudflare Pages at:

- `ezc.cade.io`

## Scope

- Cloudflare Pages project for this repo
- Custom domain binding for `ezc.cade.io`
- DNS CNAME record in `cade.io`

## Prerequisites

- OpenTofu (`tofu`) or Terraform (`terraform`)
- Cloudflare API token with least-privilege permissions

## Required token permissions

- Account: Cloudflare Pages - Edit
- Zone: Zone - Read
- Zone: DNS - Edit

## Quick start

```sh
cd infra/cloudflare
cp terraform.tfvars.example terraform.tfvars
```

Edit `infra/cloudflare/terraform.tfvars` with your values.

Then run:

```sh
export CLOUDFLARE_API_TOKEN="..."
tofu init
tofu plan
tofu apply
```

## Layout

- `infra/cloudflare/main.tf`: all Cloudflare resources for this site
- `infra/cloudflare/terraform.tfvars.example`: required variable template
- `infra/cloudflare/.terraform.lock.hcl`: provider lockfile (commit this)

## What to commit vs ignore

Commit:

- `infra/cloudflare/main.tf`
- `infra/cloudflare/terraform.tfvars.example`
- `infra/cloudflare/.terraform.lock.hcl`

Ignore (already covered by `infra/.gitignore`):

- `infra/cloudflare/terraform.tfvars`
- `infra/cloudflare/.terraform/`
- `infra/cloudflare/*.tfstate` and backups

## Notes

- This repo is configured for Cloudflare Pages infrastructure via `infra/`.
- GitHub Actions deployment is intentionally not used.
