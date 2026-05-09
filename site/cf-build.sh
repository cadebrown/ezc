#!/usr/bin/env bash
# Cloudflare Pages build entry point.
#
# Cloudflare Pages build images include cargo + rustc but not wasm-pack
# or mdbook. We download both as release binaries, prepend them to PATH,
# then delegate to site/build.sh.
#
# Pinning versions keeps builds reproducible.

set -euo pipefail

WASM_PACK_VERSION="0.13.1"
MDBOOK_VERSION="0.4.40"

# Where to stash binaries for this build (added to PATH).
TOOLS="$PWD/.cf-tools"
mkdir -p "$TOOLS"
export PATH="$TOOLS:$PATH"

echo "→ Cloudflare Pages build: installing pinned tools"

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
