#!/usr/bin/env bash
# idprobe.sh -- is an `id` toolchain present, and does it have what the port
# needs?
#
# tests/run.sh compiles real `id` (tests/lowering/, the c2id front end) and so
# needs `../idc`, the toolchain submodule beside this one. The systems
# extensions the port depends on -- `word`, `alloc`, `peek*`/`poke*` -- live on
# a branch of it (see docs/ID_EXTENSIONS.md). A checkout without them will not
# fail with "missing feature"; it will fail with a parse error deep inside a
# test, which reads like this repository is broken when it is not.
#
# So probe before running: compile a five-line program that uses exactly those
# primitives and check that it prints the right number.
#
#   tools/idprobe.sh          quiet unless something is wrong; exit 0 = usable
#   tools/idprobe.sh -v       say what was found either way
#
# Exit codes:  0 usable   1 present but lacks the systems extensions
#              2 no id toolchain at all

set -uo pipefail

here=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
idc=${IDC:-$here/../idc/bin/idc}
verbose=0
[[ ${1:-} == -v ]] && verbose=1

say() { (( verbose )) && echo "idprobe: $*"; return 0; }

if [[ ! -x $idc ]]; then
    echo "idprobe: no id compiler at $idc" >&2
    echo "  this repository is a submodule of id_development and builds with" >&2
    echo "  the toolchain beside it. From the umbrella checkout:" >&2
    echo "    git submodule update --init idc" >&2
    echo "  or point IDC at an existing bin/idc." >&2
    exit 2
fi

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

# The probe exercises the four things the port cannot be written without: the
# `word` type, the flat store, and a store/load round trip through it.
# Deliberately two functions of two and three actions, with every value that
# takes a step named: a compiler that lacks the extensions must fail on
# `word`/`alloc`, not on id's action limit or its naming rules, or the
# diagnostic below would be pointing at the wrong thing.
cat > "$work/probe.id" <<'ID'
main(int argc, string[] argv) {
  word got = probe_word();
  print("" + got);
} return int 0;

probe_word() {
  word m = alloc(16);
  poke64(m + 8, 41);
  word v = peek64(m + 8) + 1;
} return word v;
ID

if ! "$idc" "$work/probe.id" -o "$work/probe" >"$work/err" 2>&1; then
    echo "idprobe: the id compiler at $idc does not accept word/alloc/peek64" >&2
    sed 's/^/  /' "$work/err" | head -5 >&2
    echo "  These are docs/ID_EXTENSIONS.md's systems extensions; they live on" >&2
    echo "  branch kernel-port/systems-extensions of the id repository." >&2
    exit 1
fi

got=$("$work/probe" 2>&1)
if [[ $got != "42" ]]; then
    echo "idprobe: the id compiler built the probe but it printed '$got', not 42" >&2
    exit 1
fi

say "usable id toolchain at $idc (word, alloc, peek64/poke64 all work)"
exit 0
