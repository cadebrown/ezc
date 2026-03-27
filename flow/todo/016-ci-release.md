# 016: CI / Release Automation

Status: todo
Created: 2026-03-26

## Context

GitHub Actions: CI (test, clippy, fmt), release (cross-platform binaries, VSIX, WASM).

## Plan

1. .github/workflows/ci.yml — test + clippy + fmt check
2. .github/workflows/release.yml — binaries + VSIX + playground deploy
3. Homebrew formula / cargo-binstall metadata
