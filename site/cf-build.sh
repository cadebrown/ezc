#!/usr/bin/env bash
# Cloudflare Pages build entry point.
#
# Cloudflare's default build image is bare — no rustc, cargo, wasm-pack,
# or mdbook. We install all four with pinned versions, then delegate to
# site/build.sh.
#
# Pinning keeps builds reproducible. Rust comes via rustup; the others
# are downloaded as prebuilt release binaries to keep the install fast.

set -euo pipefail

RUST_TOOLCHAIN="1.85.0"        # matches workspace.package.rust-version
WASM_PACK_VERSION="0.13.1"
MDBOOK_VERSION="0.4.40"

TOOLS="$PWD/.cf-tools"
mkdir -p "$TOOLS"
export PATH="$TOOLS:$HOME/.cargo/bin:$PATH"

echo "→ Cloudflare Pages build: installing pinned tools"

# Install Rust via rustup (minimal profile) if cargo isn't available.
# `--profile minimal` skips docs and clippy; we just need rustc + cargo.
if ! command -v cargo >/dev/null; then
  echo "  installing rustup + rust ${RUST_TOOLCHAIN}"
  curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs \
    | sh -s -- -y --default-toolchain "$RUST_TOOLCHAIN" --profile minimal
  # rustup writes to ~/.cargo; PATH already includes that directory.
fi

# wasm-pack needs the wasm32 target.
if ! rustup target list --installed 2>/dev/null | grep -q '^wasm32-unknown-unknown$'; then
  echo "  adding wasm32-unknown-unknown target"
  rustup target add wasm32-unknown-unknown
fi

if ! command -v wasm-pack >/dev/null; then
  echo "  installing wasm-pack v${WASM_PACK_VERSION}"
  WP_TARBALL="wasm-pack-v${WASM_PACK_VERSION}-x86_64-unknown-linux-musl"
  curl -sSL "https://github.com/rustwasm/wasm-pack/releases/download/v${WASM_PACK_VERSION}/${WP_TARBALL}.tar.gz" \
    | tar -xz -C "$TOOLS" --strip-components=1 "${WP_TARBALL}/wasm-pack"
  chmod +x "$TOOLS/wasm-pack"
fi

if ! command -v mdbook >/dev/null; then
  echo "  installing mdbook v${MDBOOK_VERSION}"
  curl -sSL "https://github.com/rust-lang/mdBook/releases/download/v${MDBOOK_VERSION}/mdbook-v${MDBOOK_VERSION}-x86_64-unknown-linux-gnu.tar.gz" \
    | tar -xz -C "$TOOLS"
  chmod +x "$TOOLS/mdbook"
fi

echo "→ Versions:"
echo "  rustc:     $(rustc --version 2>/dev/null || echo 'missing')"
echo "  cargo:     $(cargo --version 2>/dev/null || echo 'missing')"
echo "  wasm-pack: $(wasm-pack --version 2>/dev/null || echo 'missing')"
echo "  mdbook:    $(mdbook --version 2>/dev/null || echo 'missing')"

bash site/build.sh
