#!/usr/bin/env bash
# tests/run.sh -- the differential test harness.
#
# For every C source in tests/c/:
#   1. compile it with cc, run it, record stdout + exit status  (the oracle)
#   2. translate it with c2id, build the id with idc, run it
#   3. require the two to agree byte for byte, standard output and standard
#      error each on its own, so a write to the wrong stream is a difference
#
# A case that c2id cannot yet translate is reported as SKIP, not PASS -- the
# whole point of this harness is that the status ledger in docs/STATUS.md
# never gets ahead of the evidence. A case NAME.c may have a NAME.in (its standard
# input), a NAME.trap (the program must exit 1 and say the lines on stderr; the C
# oracle is not run) or a NAME.refuse (lines the refusal must contain: the case then passes
# only if c2id fails and says them).
#
# usage: tests/run.sh [name-substring ...]

set -uo pipefail

here=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
work=$here/build/tests
idc=${IDC:-$here/../idc/bin/idc}
c2id=${C2ID_SH:-$here/tools/c2id.sh}

mkdir -p "$work"

pass=0 fail=0 skip=0
failed_names=()

filter=("$@")
matches() {
    [[ ${#filter[@]} -eq 0 ]] && return 0
    local f
    for f in "${filter[@]}"; do [[ $1 == *"$f"* ]] && return 0; done
    return 1
}

# ---- structural lint: id's layout rules on the source trees, checked without
# compiling so that layout mistakes in a half-written module surface early.
if "$idc" "$here/c2id" --check >"$work/lint.out" 2>&1 && "$idc" "$here/crt" --check >>"$work/lint.out" 2>&1; then
    echo "PASS  layout-rule-of-3"
    pass=$((pass + 1))
else
    echo "FAIL  layout-rule-of-3"
    grep -v ': warning:' "$work/lint.out" | sed 's/^/    /' | head -10
    fail=$((fail + 1)); failed_names+=("layout-rule-of-3")
fi

# ---- the front end: real lexer + expression parser + statement parser over
# real C. Proves the modules integrate, not just that each compiles alone.
if "$here/tests/frontend/build.sh" >"$work/fe.err" 2>&1; then
    got=$(printf 'if (p->len > 0) { total += arr[i].n; } else { return -1; }\n' \
          | "$here/build/frontend/parse")
    want='0: (if  a3 (block  a0 (expr  a9)) (block  a0 (return  a13)))'
    if [[ $got == "$want" ]]; then
        echo "PASS  frontend-integration"
        pass=$((pass + 1))
    else
        echo "FAIL  frontend-integration: got"
        echo "    $got"
        echo "  wanted"
        echo "    $want"
        fail=$((fail + 1)); failed_names+=("frontend-integration")
    fi
else
    echo "FAIL  frontend-integration: harness does not build"
    sed 's/^/    /' "$work/fe.err" | head -5
    fail=$((fail + 1)); failed_names+=("frontend-integration")
fi

# ---- the lowering reference: the hand-written project in tests/lowering/ is
# the exact shape c2id must generate, so it has to keep compiling and keep
# printing 55 whatever else changes.
#
# This used to be built with idc.py, because idc.py was once the only compiler
# that enforced the action, nesting and uniqueness rules. bin/idc enforces all
# of those and two more idc.py never learned, so bin/idc is the stricter check.
# What went with idc.py is a second, independent implementation agreeing that
# the project is legal.
if "$idc" "$here/tests/lowering" --allow-untested -o "$work/lowering" >"$work/lowering.err" 2>&1; then
    got=$("$work/lowering")
    if [[ $got == "55" ]]; then
        echo "PASS  lowering-reference"
        pass=$((pass + 1))
    else
        echo "FAIL  lowering-reference: printed '$got', expected '55'"
        fail=$((fail + 1)); failed_names+=("lowering-reference")
    fi
else
    echo "FAIL  lowering-reference: does not compile under bin/idc"
    sed 's/^/    /' "$work/lowering.err" | head -5
    fail=$((fail + 1)); failed_names+=("lowering-reference")
fi

for src in "$here"/tests/c/*.c; do
    name=$(basename "$src" .c)
    matches "$name" || continue

    # A NAME.cflags file holds extra flags for the oracle; -funsigned-char among
    # them makes c2id's plain char unsigned too (C2ID_UNSIGNED_CHAR, which the
    # kernel's preprocessed files need, since they do not record the flag).
    cflags=""
    [[ -f ${src%.c}.cflags ]] && cflags=$(< "${src%.c}.cflags")
    uchar=0
    [[ $cflags == *-funsigned-char* ]] && uchar=1
    export C2ID_UNSIGNED_CHAR=$uchar

    # ---- 1. the oracle: what the real C compiler does
    # shellcheck disable=SC2086
    if ! cc -std=gnu11 -w $cflags -o "$work/$name.ref" "$src" -lm 2>"$work/$name.ccerr"; then
        echo "ERROR $name: reference C does not compile"
        sed 's/^/    /' "$work/$name.ccerr" | head -5
        fail=$((fail + 1)); failed_names+=("$name"); continue
    fi

    # A case with a .refuse file is a program c2id must refuse: the translation
    # has to fail, and every line of the file has to appear in what it said. It
    # is not run, since the point is that no id program comes out of it.
    if [[ -f ${src%.c}.refuse ]]; then
        if [[ ! -x $c2id ]]; then
            skip=$((skip + 1)); continue
        fi
        rm -rf "$work/$name.id"
        if "$c2id" "$src" "$work/$name.id" > "$work/$name.c2iderr" 2>&1; then
            echo "FAIL  $name: c2id translated a program it has to refuse"
            fail=$((fail + 1)); failed_names+=("$name"); continue
        fi
        missing=""
        while IFS= read -r want; do
            [[ -z $want ]] && continue
            grep -qF -- "$want" "$work/$name.c2iderr" || missing="$missing [$want]"
        done < "${src%.c}.refuse"
        if [[ -n $missing ]]; then
            echo "FAIL  $name: the refusal does not say:$missing"
            sed 's/^/    /' "$work/$name.c2iderr" | head -6
            fail=$((fail + 1)); failed_names+=("$name"); continue
        fi
        echo "PASS  $name (refused)"
        pass=$((pass + 1)); continue
    fi

    # A case with a .trap file is a program that has no defined C behaviour (it
    # recurses forever): its oracle is not run. The port has to be translated
    # and built, exit with status 1, and say every line of the file on stderr.
    if [[ -f ${src%.c}.trap ]]; then
        rm -rf "$work/$name.id"
        if ! "$c2id" "$src" "$work/$name.id" > "$work/$name.c2iderr" 2>&1 \
            || ! "$idc" "$work/$name.id" --allow-untested -o "$work/$name.out" > "$work/$name.idcerr" 2>&1; then
            echo "FAIL  $name: does not translate and build"
            fail=$((fail + 1)); failed_names+=("$name"); continue
        fi
        "$work/$name.out" > "$work/$name.gotout" 2> "$work/$name.goterr" < /dev/null
        gotstatus=$?
        missing=""
        while IFS= read -r want; do
            [[ -z $want ]] && continue
            grep -qF -- "$want" "$work/$name.goterr" || missing="$missing [$want]"
        done < "${src%.c}.trap"
        if [[ $gotstatus -ne 1 || -n $missing ]]; then
            echo "FAIL  $name: exit status $gotstatus (expected 1), stderr lacks:$missing"
            fail=$((fail + 1)); failed_names+=("$name"); continue
        fi
        echo "PASS  $name (trapped)"
        pass=$((pass + 1)); continue
    fi

    # A .in file is the program's standard input and a .args file its arguments,
    # one per line, for the oracle and the port alike.
    stdin_file=/dev/null
    [[ -f ${src%.c}.in ]] && stdin_file=${src%.c}.in
    args=()
    [[ -f ${src%.c}.args ]] && mapfile -t args < "${src%.c}.args"
    "$work/$name.ref" "${args[@]}" > "$work/$name.refout" 2> "$work/$name.referr" < "$stdin_file"
    refstatus=$?

    # ---- 2. the port: C -> id -> binary
    if [[ ! -x $c2id ]]; then
        skip=$((skip + 1)); continue
    fi
    rm -rf "$work/$name.id"
    if ! "$c2id" "$src" "$work/$name.id" > "$work/$name.c2iderr" 2>&1; then
        echo "SKIP  $name: c2id: $(head -1 "$work/$name.c2iderr")"
        skip=$((skip + 1)); continue
    fi
    if ! "$idc" "$work/$name.id" --allow-untested -o "$work/$name.out" > "$work/$name.idcerr" 2>&1; then
        echo "FAIL  $name: generated id does not compile"
        sed 's/^/    /' "$work/$name.idcerr" | head -5
        fail=$((fail + 1)); failed_names+=("$name"); continue
    fi
    "$work/$name.out" "${args[@]}" > "$work/$name.gotout" 2> "$work/$name.goterr" < "$stdin_file"
    gotstatus=$?

    # ---- 3. compare
    if [[ $refstatus -ne $gotstatus ]]; then
        echo "FAIL  $name: exit status $gotstatus, expected $refstatus"
        fail=$((fail + 1)); failed_names+=("$name"); continue
    fi
    if ! diff -u "$work/$name.refout" "$work/$name.gotout" > "$work/$name.diff" || ! diff -u "$work/$name.referr" "$work/$name.goterr" >> "$work/$name.diff"; then
        echo "FAIL  $name: output differs"
        sed 's/^/    /' "$work/$name.diff" | head -12
        fail=$((fail + 1)); failed_names+=("$name"); continue
    fi
    echo "PASS  $name"
    pass=$((pass + 1))
done

echo
echo "$pass passed, $fail failed, $skip skipped"
[[ $skip -gt 0 && $pass -eq 0 ]] && echo "(c2id not built yet -- see docs/STATUS.md)"
if [[ $fail -gt 0 ]]; then
    echo "failed: ${failed_names[*]}"
    exit 1
fi
exit 0
