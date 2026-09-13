#!/usr/bin/env bash
# check.sh -- everything a developer should run before committing.
#
# One entry point, so "did I break anything?" has a single answer. Each step
# either passes, fails, or is *skipped with a reason* -- and a skip is printed
# loudly rather than folded into the pass count, because a check that quietly
# does nothing is worse than no check at all.
#
#   tools/check.sh                  every check (seconds)
#   tools/check.sh --require-all    treat a skipped step as a failure; this is
#                                   what a fully provisioned machine should use
#
# The steps:
#   1. tools/lint3.sh          id's rule of 3 on c2id/ and crt/
#   2. tools/idprobe.sh        is there a usable id toolchain?          (gate)
#   3. tests/crt/run.sh        crt against the same calls in C (needs step 2)
#   4. tests/run.sh            the differential harness       (needs step 2)
#
# Measuring how far the kernel port gets is not here: that needs a kernel tree
# and lives in linux_id, which consumes this compiler rather than containing
# it.

set -uo pipefail

here=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
require_all=0

while [[ $# -gt 0 ]]; do
    case $1 in
        --require-all) require_all=1; shift ;;
        -h|--help) sed -n '2,20p' "${BASH_SOURCE[0]}"; exit 0 ;;
        *) echo "check: unknown argument: $1" >&2; exit 2 ;;
    esac
done

pass=0 fail=0 skip=0
failed=()
skipped=()

rule() { printf '\n== %s\n' "$1"; }
ok()   { pass=$((pass + 1)); printf 'ok    %s\n' "$1"; }
bad()  { fail=$((fail + 1)); failed+=("$1"); printf 'FAIL  %s\n' "$1"; }
meh()  { skip=$((skip + 1)); skipped+=("$1: $2"); printf 'SKIP  %s -- %s\n' "$1" "$2"; }

# ---- 1. structural lint
rule "tools/lint3.sh -- id's rule of 3"
if "$here/tools/lint3.sh"; then ok layout; else bad layout; fi

# ---- 2. the id toolchain. tests/run.sh compiles real id, so without a usable
# compiler it cannot run at all -- and saying so is far better than letting it
# fail with a parse error that looks like this repository's fault.
rule "tools/idprobe.sh -- id toolchain"
idprobe_out=$("$here/tools/idprobe.sh" -v 2>&1); idprobe_rc=$?
echo "$idprobe_out"
case $idprobe_rc in
    0) ok id-toolchain ;;
    1) bad id-toolchain ;;   # present but wrong: that is a real problem
    *) meh id-toolchain "no id compiler; set IDC or clone id_development" ;;
esac

# ---- 3. crt's behaviour, against C
#
# crt is C's semantics written in id, so the only test of it that means
# anything is the one that asks C. This was missing for a long time: the
# compiler could say whether crt was legal id and nothing could say whether it
# still did what C does.
rule "tests/crt/run.sh -- crt against the same calls in C"
if [[ $idprobe_rc -eq 0 ]]; then
    if "$here/tests/crt/run.sh"; then ok crt; else bad crt; fi
else
    meh crt "needs the id toolchain (see above)"
fi

# ---- 4. the harness
rule "tests/run.sh -- the differential harness"
if [[ $idprobe_rc -eq 0 ]]; then
    if "$here/tests/run.sh"; then ok tests; else bad tests; fi
else
    meh tests "needs the id toolchain (see above)"
fi

# ---- verdict
printf '\n%s\n' "-------------------------------------------------"
printf '%d passed, %d failed, %d skipped\n' "$pass" "$fail" "$skip"
(( skip )) && printf 'skipped:\n' && printf '  %s\n' "${skipped[@]}"
if (( fail )); then
    printf 'failed: %s\n' "${failed[*]}"
    exit 1
fi
if (( skip && require_all )); then
    printf 'check: --require-all was given and %d step(s) were skipped\n' "$skip"
    exit 1
fi
exit 0
