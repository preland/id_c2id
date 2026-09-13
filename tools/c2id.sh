#!/usr/bin/env bash
# c2id.sh -- run the C-to-id compiler over a translation unit.
#
#   tools/c2id.sh <input.c|input.i> <output-project-dir>
#
# c2id itself is written in id, and id has no filesystem builtins, so it reads
# preprocessed C on stdin and writes the whole output project to stdout as a
# stream of tagged files:
#
#     ==== FILE b/0/0/0/0/0/f0.id
#     <contents>
#     ==== FILE b/0/0/0/0/1/f0.id
#     <contents>
#
# Splitting that stream into real files is this script's only job -- the same
# bootstrap-layer bargain id itself makes with bin/idc.
#
# A plain .c input is preprocessed first; a .i input is used as-is.

set -euo pipefail

here=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
c2id_bin=${C2ID:-$here/build/c2id}
idc=${IDC:-$here/../idc/bin/idc}

if [[ $# -ne 2 ]]; then
    echo "usage: ${BASH_SOURCE[0]} <input.c|input.i> <output-project-dir>" >&2
    exit 2
fi
src=$1
out=$2

# Build c2id itself if it isn't there. It is an id project like any other.
#
# bin/idc, the compiler every other id program is built with. This was idc.py
# for a long time, because c2id was written against idc.py and broke two rules
# in docs/SPEC.md that only bin/idc enforces -- a call may not be an argument to
# a call, and a return clause is a name or a literal. c2id obeys both now (the
# umbrella's docs/GAPS.md B6/B7), so the translator and the projects it emits are
# held to the same language.
if [[ ! -x $c2id_bin ]]; then
    mkdir -p "$(dirname "$c2id_bin")"
    echo "c2id: building the compiler from c2id/ ..." >&2
    "$idc" "$here/c2id" --allow-untested -o "$c2id_bin" >&2
fi

# Preprocess if needed. -P drops the linemarkers only for plain .c inputs
# compiled standalone; kernel inputs come in as .i from tools/kcpp.sh with
# their markers intact, which is how c2id tells kernel code from header noise.
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
if [[ $src == *.i ]]; then
    cp "$src" "$work/in.i"
else
    cc -std=gnu11 -E "$src" -o "$work/in.i"
fi

"$c2id_bin" < "$work/in.i" > "$work/out.txt"

# Split the tagged stream. Everything after a "==== FILE <path>" line belongs
# to that file, until the next such line. Anything before the first tag is a
# diagnostic, not output, and goes to stderr unless it is blank.
#
# Each line is held until the next one arrives, because only then is it known
# not to be the stream's last: awk cannot see whether the last line ended in a
# newline, so the shell looks and says so in lastnl, and the file gets exactly
# the bytes the stream had. A tag closes the file before it, so a path named
# twice is rewritten from empty, as opening it for writing would.
rm -rf "$out"
mkdir -p "$out"
lastnl=1
if [[ -s $work/out.txt && -n $(tail -c 1 "$work/out.txt") ]]; then
    lastnl=0
fi
awk -v outdir="$out" -v lastnl="$lastnl" '
function put(eol) {
    if (have) {
        printf "%s%s", held, eol > path
    } else if (held ~ /[^ \t\n\r\f\v]/) {
        printf "%s%s", held, eol | "cat 1>&2"
    }
}
function open_file(name,    dir) {
    if (have) close(path)
    path = outdir "/" name
    dir = path
    sub(/\/[^\/]*$/, "", dir)
    gsub(/\047/, "\047\\\047\047", dir)
    system("mkdir -p \047" dir "\047")
    printf "" > path
    have = 1
    count++
}
{
    if (pending) put("\n")
    pending = 0
    if (index($0, "==== FILE ") == 1) {
        name = substr($0, 11)
        sub(/^[ \t\r\f\v]+/, "", name)
        sub(/[ \t\r\f\v]+$/, "", name)
        open_file(name)
    } else {
        held = $0
        pending = 1
    }
}
END {
    if (pending) put(lastnl ? "\n" : "")
    if (have) close(path)
    close("cat 1>&2")
    if (count == 0) {
        print "c2id: produced no files" | "cat 1>&2"
        exit 1
    }
    print "c2id: wrote " count " id files to " outdir | "cat 1>&2"
}' "$work/out.txt"

# The generated project needs the C-semantics runtime compiled alongside it.
cp -r "$here/crt" "$out/crt"
# conf.id is only read at a root, so crt's constants move up to the project's.
mv "$out/crt/conf.id" "$out/conf.id"
echo "$out"
