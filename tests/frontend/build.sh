#!/usr/bin/env bash
# Build the front-end integration harness: the real lexer + expression parser
# + statement parser, with a tree printer on top.
#
# The parser is not a slice that stands alone any more: it reads the diagnostics
# (front/diag), the type sizes the emitter owns and the constants the folder
# needs from the middle and the back end, so the harness is c2id itself with its
# driver (top/main.id) swapped for harn/main.id.
set -euo pipefail
here=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
idc=${IDC:-$here/../idc/bin/idc}
work=$here/build/frontend
rm -rf "$work/src"
mkdir -p "$work/src"
cp "$here/c2id/conf.id" "$work/src/conf.id"
cp -r "$here/c2id/front" "$work/src/front"
cp -r "$here/c2id/mid" "$work/src/mid"
cp -r "$here/c2id/back" "$work/src/back"
rm "$work/src/back/emit/gen/out/top/main.id"
cp -r "$here/tests/frontend/harn" "$work/src/harn"
"$idc" "$work/src" --allow-untested -o "$work/parse"
echo "$work/parse"
