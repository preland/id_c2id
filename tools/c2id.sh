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
idc_py=${IDC_PY:-$here/../idc/idc.py}

if [[ $# -ne 2 ]]; then
    echo "usage: ${BASH_SOURCE[0]} <input.c|input.i> <output-project-dir>" >&2
    exit 2
fi
src=$1
out=$2

# Build c2id itself if it isn't there. It is an id project like any other.
#
# idc.py rather than bin/idc, and that is a debt rather than a preference. The
# two compilers disagree about two rules in docs/SPEC.md -- a call may not be an
# argument to a call, and a return clause is a name or a literal -- which
# bin/idc enforces and idc.py does not. c2id was written against idc.py and so
# breaks both, 518 times; see the umbrella's docs/GAPS.md B6/B7. Until that is
# paid off, switching this line to bin/idc does not build.
#
# What idc.py does enforce, and what this line was originally here for, is the
# rest: the rule of 3, one name one type, function-logic uniqueness. c2id obeys
# those like anything else.
if [[ ! -x $c2id_bin ]]; then
    mkdir -p "$(dirname "$c2id_bin")"
    echo "c2id: building the compiler from c2id/ ..." >&2
    python3 "$idc_py" "$here/c2id" -o "$c2id_bin" >&2
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
# to that file, until the next such line.
rm -rf "$out"
mkdir -p "$out"
python3 - "$work/out.txt" "$out" <<'PY'
import os, sys
stream, outdir = sys.argv[1], sys.argv[2]
cur, buf, count = None, [], 0

def flush():
    global buf, count
    if cur is None:
        return
    path = os.path.join(outdir, cur)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w") as f:
        f.write("".join(buf))
    buf = []
    count += 1

with open(stream) as f:
    for line in f:
        if line.startswith("==== FILE "):
            flush()
            cur = line[len("==== FILE "):].strip()
        elif cur is not None:
            buf.append(line)
        elif line.strip():
            # Anything before the first tag is a diagnostic, not output.
            sys.stderr.write(line)
flush()
if count == 0:
    sys.stderr.write("c2id: produced no files\n")
    sys.exit(1)
print(f"c2id: wrote {count} id files to {outdir}", file=sys.stderr)
PY

# The generated project needs the C-semantics runtime compiled alongside it.
cp -r "$here/crt" "$out/crt"
echo "$out"
