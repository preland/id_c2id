#!/usr/bin/env bash
# crt behaviour: the same calls in `id` and in C, and the two outputs compared.
#
# There was no such test. docs/STATUS.md claimed crt's output had been diffed
# against C across 44 cases and no command in this repository did that, so crt
# had a compiler to tell it whether it was legal `id` and nothing at all to tell
# it whether it still behaved like C. This is a smoke test rather than those 44
# cases, and it is honest about which lines C is the oracle for.
#
# Three lines are crt's own promise rather than C's and are checked against a
# stated golden instead of against cc:
#
#   unk=%q      an unimplemented specifier is echoed, not dropped. What glibc
#               does with one is unspecified.
#   apart 1     two live 64-byte blocks do not overlap.
#   reused 1    a freed block of the same size comes straight back.
#
# The allocator's addresses are never compared with C's: C's malloc is not this
# allocator. The invariant is compared; the address is not.
#
# Run from anywhere: tests/crt/run.sh
set -u
cd "$(dirname "$0")"
HERE=$(pwd)
ROOT=$(cd ../.. && pwd)
IDC=${IDC:-$ROOT/../idc/bin/idc}
pass=0 fail=0
ok()  { pass=$((pass+1)); echo "PASS: $1"; }
bad() { fail=$((fail+1)); echo "FAIL: $1"; }

TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT

# crt has no main, so the program and crt are built as one project -- the same
# shape tools/c2id.sh gives generated code.
mkdir -p "$TMP/p"
cp -r "$HERE/prog/." "$TMP/p/"
cp -r "$ROOT/crt" "$TMP/p/crt"
# conf.id is only read at a root, so crt's constants move up to the project's.
mv "$TMP/p/crt/conf.id" "$TMP/p/conf.id"

if "$IDC" "$TMP/p" --allow-untested -o "$TMP/idprog" >"$TMP/build.log" 2>&1; then
    ok "crt builds with the primary compiler"
else
    bad "crt builds with the primary compiler ($(head -1 "$TMP/build.log"))"
    echo; echo "$pass passed, $fail failed"; exit 1
fi

# This used to build the same project with idc.py as well. idc.py enforces
# nothing bin/idc does not, so what it added was a second implementation's
# agreement. The check that replaces it is crt alone, with no program: every
# rule is still checked on every function, and nothing in prog/ can be what
# makes crt legal.
if "$IDC" "$ROOT/crt" --allow-untested --emit-c "$TMP/crt.c" >"$TMP/build2.log" 2>&1; then
    ok "crt builds on its own with the primary compiler"
else
    bad "crt builds on its own with the primary compiler ($(head -1 "$TMP/build2.log"))"
fi

if ! cc -fno-builtin -o "$TMP/twin" "$HERE/twin.c" 2>"$TMP/cc.log"; then
    bad "the C twin compiles ($(head -1 "$TMP/cc.log"))"
    echo; echo "$pass passed, $fail failed"; exit 1
fi
ok "the C twin compiles"

"$TMP/idprog" > "$TMP/id.out" 2>&1
"$TMP/twin"   > "$TMP/c.out"  2>&1

# The lines C is the oracle for.
grep -Ev '^(unk=|apart |reused )' "$TMP/id.out" > "$TMP/id.cmp"
if diff -u "$TMP/c.out" "$TMP/id.cmp" > "$TMP/d.txt"; then
    ok "crt's printf, strlen, strcmp and memset agree with C"
else
    bad "crt's printf, strlen, strcmp and memset agree with C"
    sed -n '1,20p' "$TMP/d.txt"
fi

# The lines that are crt's own promise.
want='unk=%q
apart 1
reused 1'
got=$(grep -E '^(unk=|apart |reused )' "$TMP/id.out")
if [ "$got" = "$want" ]; then
    ok "the allocator's invariants hold and an unknown specifier is echoed"
else
    bad "the allocator's invariants hold and an unknown specifier is echoed: got '$got'"
fi

echo
echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
