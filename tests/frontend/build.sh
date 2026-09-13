#!/usr/bin/env bash
# Build the front-end integration harness: the real lexer + expression parser
# + statement parser, plus a stub type module and a tree printer.
set -euo pipefail
here=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
idc=${IDC:-$here/../idc/bin/idc}
work=$here/build/frontend
rm -rf "$work/src"
mkdir -p "$work/src/c2id"
cp -r "$here/c2id/lex" "$work/src/c2id/lex"
cp -r "$here/c2id/parse" "$work/src/c2id/parse"
cp -r "$here/tests/frontend/harn" "$work/src/harn"
"$idc" "$work/src" -o "$work/parse"
echo "$work/parse"
