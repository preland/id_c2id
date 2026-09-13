#!/usr/bin/env bash
# lint3.sh -- check id's structural rules on a source tree without compiling.
#
# bin/idc enforces these too, but only once the whole project compiles. While
# modules are being written independently this catches the layout mistakes
# early, which matters because fixing a directory level after the fact means
# moving everything below it.
#
# usage: tools/lint3.sh [dir ...]     (default: c2id crt)
set -uo pipefail
here=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
roots=("${@:-}")
[[ -z ${roots[0]:-} ]] && roots=("$here/c2id" "$here/crt")

bad=0
for root in "${roots[@]}"; do
    [[ -d $root ]] || continue
    while IFS= read -r d; do
        # Only .id files and subdirectories count; .md files are free, and so
        # is conf.id, which bin/idc reads as a manifest rather than source.
        n_id=$(find "$d" -maxdepth 1 -name '*.id' ! -name conf.id | wc -l)
        n_dir=$(find "$d" -mindepth 1 -maxdepth 1 -type d | wc -l)
        total=$((n_id + n_dir))
        if (( total > 3 )); then
            echo "TOO MANY ENTRIES ($total): ${d#$here/}"
            bad=$((bad + 1))
        fi
    done < <(find "$root" -type d)

    while IFS= read -r f; do
        # A function definition is a line starting at column 0 with `name(`.
        nf=$(grep -cE '^[A-Za-z_][A-Za-z0-9_]*\(' "$f")
        if (( nf > 3 )); then
            echo "TOO MANY FUNCTIONS ($nf): ${f#$here/}"
            bad=$((bad + 1))
        fi
    done < <(find "$root" -name '*.id')
done

if (( bad )); then
    echo "$bad structural violation(s)"
    exit 1
fi
echo "layout ok: $(find "${roots[@]}" -name '*.id' 2>/dev/null | wc -l) id files"
