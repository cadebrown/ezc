terraform {
  required_version = ">= 1.6.0"

  required_providers {
    cloudflare = {
      source  = "cloudflare/cloudflare"
      version = "~> 4.0"
    }
  }
}

variable "account_id" {
  description = "Cloudflare account ID that owns the zone and Pages project."
  type        = string
}

variable "zone_name" {
  description = "Zone apex for DNS management."
  type        = string
  default     = "cade.io"
}

variable "pages_project_name" {
  description = "Cloudflare Pages project name."
  type        = string
  default     = "ezc"
}

variable "pages_custom_domain" {
  description = "Custom domain bound to this Pages project."
  type        = string
  default     = "ezc.cade.io"

  validation {
    condition = (
      var.pages_custom_domain == var.zone_name ||
      endswith(var.pages_custom_domain, ".${var.zone_name}")
    )
    error_message = "pages_custom_domain must be the zone apex or a subdomain of zone_name."
  }
}

variable "pages_production_branch" {
  description = "Git branch used for production deployments."
  type        = string
  default     = "main"
}

variable "github_owner" {
  description = "GitHub user/org that owns this repository."
  type        = string
}

variable "github_repo" {
  description = "GitHub repository name connected to Cloudflare Pages."
  type        = string
  default     = "ezc"
}

variable "pages_build_command" {
  description = "Build command executed by Cloudflare Pages."
  type        = string
  default     = "if ! command -v cargo >/dev/null 2>&1; then curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --profile minimal --default-toolchain stable; fi && . \"$HOME/.cargo/env\" && rustup target add wasm32-unknown-unknown && cargo install dioxus-cli --version 0.7.2 --locked && cargo install wasm-bindgen-cli --locked && dx build -p ezcweb --web --release --debug-symbols false"
}

variable "pages_destination_dir" {
  description = "Output directory produced by the build command (relative to root_dir)."
  type        = string
  default     = "target/dx/ezcweb/release/web/public"
}

variable "pages_root_dir" {
  description = "Root directory for the Pages build (empty string means repository root)."
  type        = string
  default     = ""
}

provider "cloudflare" {}

data "cloudflare_zone" "site" {
  name = var.zone_name
}

locals {
  pages_dns_record_name = var.pages_custom_domain == var.zone_name ? "@" : trimsuffix(var.pages_custom_domain, ".${var.zone_name}")
}

resource "cloudflare_pages_project" "site" {
  account_id        = var.account_id
  name              = var.pages_project_name
  production_branch = var.pages_production_branch

  build_config {
    build_command   = var.pages_build_command
    destination_dir = var.pages_destination_dir
    root_dir        = var.pages_root_dir
  }

  source {
    type = "github"
    config {
      owner             = var.github_owner
      repo_name         = var.github_repo
      production_branch = var.pages_production_branch
    }
  }
}

resource "cloudflare_pages_domain" "site" {
  account_id   = var.account_id
  project_name = cloudflare_pages_project.site.name
  domain       = var.pages_custom_domain
}

resource "cloudflare_record" "site" {
  zone_id = data.cloudflare_zone.site.id
  type    = "CNAME"
  name    = local.pages_dns_record_name
  content = cloudflare_pages_project.site.subdomain
  proxied = true
  ttl     = 1
}

output "pages_project_name" {
  value = cloudflare_pages_project.site.name
}

output "pages_project_subdomain" {
  value = cloudflare_pages_project.site.subdomain
}

output "pages_custom_domain" {
  value = cloudflare_pages_domain.site.domain
}

output "pages_domain_status" {
  value = cloudflare_pages_domain.site.status
}
