# Making `bin/idc` the compiler

`id` exists because heavily constrained code has essentially one viable
expression, which is what keeps a codebase coherent when many different hands
work on it. A compiler written *in* `id` inherits that property; a 3,400-line
Python monofile does not. So `idc.py`'s job is stage-0 bootstrap and nothing
else, and every rule it enforces has to move into the self-hosted compiler.

This is the ledger for that move.

## Measured starting point

All 38 rule-violating programs in `id_development/tests/invalid/`, plus a
project-layout case, run through both compilers. Thirteen rules are silently
accepted by the self-hosted path:

| Group | Rule |
| --- | --- |
| **Structural** | 3 actions per block |
| | nesting depth ≤ 2 |
| | ≤ 3 functions per file |
| | ≤ 3 entries per directory |
| | function-logic uniqueness |
| **Naming / scope** | one name, one type, program-wide |
| | an exported name is reserved program-wide |
| **Type** | comparing incompatible types |
| | index-assign element type |
| | `push` element type |
| | untyped empty list literal |
| | `void[]` as a type |

And the crucial part: of the 26 cases `bin/idc` *does* reject, it rejects
**all** of them by falling back to `idc.py`. The self-hosted stages produce no
diagnostics of their own. That has a perverse consequence — as they get better
and fall back less, `bin/idc` catches *fewer* errors. The fallback is not a
safety net; it is what has been hiding the gap.

## Where each check has to live

`cat *.id | idlex | idparse` throws away file boundaries, and `id` has no
filesystem access at all. So the checks split three ways, and the split is
forced rather than chosen:

**In `idparse`, written in `id`** — everything that is visible in the token
stream: the action limit, nesting depth, one-name-one-type, exported-name
reservation, export/import access, the type checks, and function-logic
uniqueness.

**In the driver** — the 3-entries-per-directory rule. This is a property of
the *filesystem*, which `id` cannot see. It is the same reason `bin/idc` exists
at all, and the README already accepts a bootstrap layer for directory
walking. Counting entries there is not a retreat.

**Needs a small protocol change** — ≤ 3 functions per file. It is a per-file
rule and the pipeline has already concatenated the files by the time `idparse`
sees them. Two options:

1. the driver counts functions per file itself — cheap, but it means parsing
   `id` with `grep`, outside the language;
2. the driver emits a file marker between concatenated files, the lexer turns
   it into a token, and `idparse` counts properly.

(2) is the right answer. It keeps the rule in the language, it costs one token
kind, and the marker also gives every later diagnostic a real filename instead
of a position in a concatenated blob — which is worth having on its own.

## Order of work

1. **Syntax coverage** — done for the lexer (`word`, `<< >>`, hex literals),
   in progress for the parser (the bitwise precedence levels, `~`, the `word`
   type through the type pass and emitter, the new builtins). Until this
   lands, nothing in `linux_id` exercises the self-hosted path at all: every
   build falls back, so "using `bin/idc`" silently means running `idc.py`.
2. **Action limit and nesting depth.** The two rules that most define the
   language, both pure tree walks over the parsed function, needing no type
   information. Highest value per unit of work, and the ones that keep
   *generated* kernel code honest.
3. **File markers**, then functions-per-file; and entries-per-directory in the
   driver.
4. **One name, one type**, and exported-name reservation. `idparse` already
   keeps `vnames`/`vtypes` and an export table, so this is mostly bookkeeping.
5. **The type checks.** `idparse` already has a type pass for codegen; these
   are assertions on results it already computes.
6. **Function-logic uniqueness.** Hardest: it needs body canonicalisation with
   parameters and locals alpha-renamed. Leave it last.
7. **Delete `do_fallback`** from `bin/idc`. After that, `idc.py` runs only when
   there is no cached `idlex`/`idparse` to bootstrap from — and ideally not
   even then, since a built pair can rebuild itself.

## The honest risk

Steps 2–6 mean writing a semantic analyser in a language with three actions per
block and no records. That is precisely the exercise `id` is for, and the
existing 273-function parser is proof it is workable — but it is real work, and
each step must keep `tools/parity.sh` at MATCH, because byte-identical codegen
against `idc.py` is the only evidence that the self-hosted compiler is
correct while `idc.py` still exists to compare against. Once the fallback is
gone that evidence gets harder to obtain, so **parity should be locked in
before the fallback is removed, not after**.
