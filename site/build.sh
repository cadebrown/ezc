#!/usr/bin/env bash
# Build the ezc documentation site.
#
# Steps:
#   1. clean the previous output
#   2. wasm-pack build  → site/public/pkg/
#   3. mdbook build     → site/book/
#   4. copy public/     → book/  (excluding wasm-pack's package.json)
#
# Run from anywhere; uses the script's location as the project root.
# Requires: cargo, wasm-pack, mdbook.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(dirname "$SCRIPT_DIR")"
SITE="$SCRIPT_DIR"

require() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "✗ missing dependency: $1" >&2
    echo "  install hint: $2" >&2
    exit 1
  fi
}

require wasm-pack "cargo binstall wasm-pack"
require mdbook    "cargo binstall mdbook"

echo "→ Cleaning previous build output"
rm -rf "$SITE/book" "$SITE/public/pkg"

echo "→ Building WASM bundle"
cd "$ROOT"
if ! wasm-pack build crates/ezc-web --target web --out-dir "$SITE/public/pkg" --release; then
  echo "✗ wasm-pack failed" >&2
  exit 1
fi
# wasm-pack writes a package.json + .gitignore for npm publishing — neither
# belongs in the deployed site.
rm -f "$SITE/public/pkg/package.json" "$SITE/public/pkg/.gitignore"

echo "→ Building mdbook"
cd "$SITE"
if ! mdbook build; then
  echo "✗ mdbook failed" >&2
  exit 1
fi

echo "→ Copying WASM bundle into book output"
cp -r "$SITE/public/." "$SITE/book/"

echo "✓ Site built at $SITE/book/"
echo "  Serve with: python3 -m http.server -d $SITE/book"
