#!/usr/bin/env bash
# Build the ezc documentation site.
#
# Steps:
#   1. wasm-pack build  → site/public/pkg/
#   2. mdbook build     → site/book/
#   3. copy public/     → book/
#
# Run from anywhere; uses the script's location as the project root.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(dirname "$SCRIPT_DIR")"
SITE="$SCRIPT_DIR"

echo "→ Building WASM bundle"
cd "$ROOT"
wasm-pack build crates/ezc-web --target web --out-dir "$SITE/public/pkg" --release

echo "→ Building mdbook"
cd "$SITE"
mdbook build

echo "→ Copying static assets"
cp -r "$SITE/public/." "$SITE/book/"

echo "✓ Site built at $SITE/book/"
echo "  Serve with: python3 -m http.server -d $SITE/book"
