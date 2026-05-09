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
  description = "Existing Cloudflare zone (apex domain) to host the subdomain on."
  type        = string
  default     = "cade.io"
}

variable "pages_project_name" {
  description = "Cloudflare Pages project name."
  type        = string
  default     = "ezc-cade-io"
}

variable "subdomain_label" {
  description = "Subdomain label (e.g. 'ezc' for ezc.cade.io)."
  type        = string
  default     = "ezc"
}

variable "pages_custom_domain" {
  description = "Full custom domain to bind to the Pages project (FQDN)."
  type        = string
  default     = "ezc.cade.io"
}

variable "pages_production_branch" {
  description = "Git branch used for production deploys."
  type        = string
  default     = "main"
}

variable "github_owner" {
  description = "GitHub org or user that owns the repo."
  type        = string
}

variable "github_repo" {
  description = "GitHub repo name for the Pages project."
  type        = string
  default     = "ezc"
}

variable "pages_build_command" {
  description = "Build command run by Cloudflare Pages. Installs wasm-pack + mdbook, then invokes site/build.sh which assembles site/book/."
  type        = string
  default     = "bash site/cf-build.sh"
}

variable "pages_destination_dir" {
  description = "Output directory for the build (relative to root_dir)."
  type        = string
  default     = "site/book"
}

variable "pages_root_dir" {
  description = "Root directory for the Pages build (empty means repo root)."
  type        = string
  default     = ""
}

provider "cloudflare" {}

data "cloudflare_zone" "apex" {
  name = var.zone_name
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

resource "cloudflare_pages_domain" "subdomain" {
  account_id   = var.account_id
  project_name = cloudflare_pages_project.site.name
  domain       = var.pages_custom_domain
}

resource "cloudflare_record" "subdomain" {
  zone_id = data.cloudflare_zone.apex.id
  type    = "CNAME"
  name    = var.subdomain_label
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

output "pages_domain_status" {
  value = cloudflare_pages_domain.subdomain.status
}
