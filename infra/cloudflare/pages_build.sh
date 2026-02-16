#!/usr/bin/env bash
set -euo pipefail

DX_VERSION="0.7.2"
WASM_BINDGEN_VERSION="0.2.108"

has_version() {
  local bin="$1"
  local wanted="$2"

  if ! command -v "$bin" >/dev/null 2>&1; then
    return 1
  fi

  "$bin" --version 2>/dev/null | grep -Fq "$wanted"
}

ensure_rust() {
  if command -v cargo >/dev/null 2>&1; then
    return
  fi

  curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | \
    sh -s -- -y --profile minimal --default-toolchain stable
}

install_with_binstall_or_cargo() {
  local crate="$1"
  local version="$2"

  if command -v cargo-binstall >/dev/null 2>&1; then
    cargo binstall -y "${crate}@${version}" && return
  else
    cargo install cargo-binstall --locked >/dev/null 2>&1 || true
    if command -v cargo-binstall >/dev/null 2>&1; then
      cargo binstall -y "${crate}@${version}" && return
    fi
  fi

  cargo install "${crate}" --version "${version}" --locked
}

ensure_tooling() {
  if ! has_version dx "$DX_VERSION"; then
    install_with_binstall_or_cargo dioxus-cli "$DX_VERSION"
  fi

  if ! has_version wasm-bindgen "$WASM_BINDGEN_VERSION"; then
    install_with_binstall_or_cargo wasm-bindgen-cli "$WASM_BINDGEN_VERSION"
  fi
}

main() {
  export CARGO_BINSTALL_DISABLE_TELEMETRY=true

  ensure_rust

  # Some environments already provide cargo on PATH without rustup's env file.
  if [ -f "$HOME/.cargo/env" ]; then
    # shellcheck disable=SC1090
    . "$HOME/.cargo/env"
  fi

  rustup target add wasm32-unknown-unknown
  ensure_tooling

  dx build -p ezcweb --web --release --debug-symbols false
}

main "$@"
