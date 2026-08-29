# Status ledger

The rule for this file: a row says "works" only if a command in `tests/` runs
and passes. Everything else is "planned" or "in progress", regardless of how
much code exists.

Last updated: 2026-08-29.

## Infrastructure

| item | state | evidence |
| --- | --- | --- |
| Differential test harness | **1 known failure** | `tests/run.sh` — C oracle vs id, byte-for-byte; currently 2 passed, 1 failed (`frontend-integration`), 7 skipped — see "Known gaps" below |
| Block-function lowering | **proven** | `tests/lowering/` — compiles under idc.py, computes 55 |
| c2id driver (stream → project tree) | works | `tools/c2id.sh` |
| Pre-commit entry point | **1 known failure** | `tools/check.sh` — currently 2 passed, 1 failed, 0 skipped |
| `id` toolchain probe | works | `tools/idprobe.sh` — 0 / 1 / 2 for usable / too old / absent |
| CI | **unproven** | `.github/workflows/ci.yml` — clean under `actionlint` where checked; every command in it passes locally; never yet run on a GitHub runner |

## The compiler itself (`../idc`)

`bin/idc` is now the compiler. `idc.py` compiles nothing — it only bootstraps
`idlex`/`idparse` on a cold cache, and `do_fallback` has been deleted.

| item | state |
| --- | --- |
| All 13 of `idc.py`'s semantic rules | enforced in the self-hosted compiler |
| Syntax-error reporting | done — text and line identical to `idc.py` |
| `file:line` on every diagnostic | done, including expression-level ones |
| `asm` functions with platform triples | done; unsupported target is an error |
| Type registry (types as data) | done for every check and the widening rule |
| Guard tests that checks stay wired | 10, feeding violating programs to the real binary |
| Suite | 134 passed, 0 failed; every parity target MATCH |

Remaining, tracked in `../idc/docs/BACKENDS.md`: a per-target spelling
column, then `box`/`unbox`/`to_str` onto it, then the dispatch layer and a
second target (Python before LLVM — it exercises the interface hardest).
`c_type`'s chain names every type explicitly, so it is no longer a
correctness hazard, only a place that still has to be edited when a type is
added.

## `id` language extensions (`../idc`)

See `docs/ID_EXTENSIONS.md` for the specification and the rationale for each.

| item | `idc.py` | self-hosted | LLVM | WASM | tests |
| --- | --- | --- | --- | --- | --- |
| hex literals | done | todo | done | done | yes |
| bitwise `& \| ^ ~ << >>` on `int` | done | todo | done | done | yes |
| `word` (64-bit machine word) | done | todo | rejects | rejects | yes |
| `alloc` / `peek*` / `poke*` | done | todo | rejects | rejects | yes |
| `udiv` / `umod` / `ult` / `ushr` | done | todo | rejects | rejects | yes |
| `str_of_mem` / `mem_of_str` | done | todo | rejects | rejects | yes |

Committed on branch `kernel-port/systems-extensions` in `../idc`; full suite
there is 116 passed, 0 failed. "rejects" means the backend refuses the
feature by name rather than miscompiling it — see `docs/ID_EXTENSIONS.md` §6
for why. "todo" on the self-hosted column means `bin/idc` falls back to
`idc.py` for programs using the new syntax, which is its documented
behaviour, so nothing is blocked.

## `crt` — C-semantics runtime, in `id`

| item | state | evidence |
| --- | --- | --- |
| integer width truncation / sign extension | done | boundaries match C exactly |
| `memcpy` / `memmove` / `memset` / `memcmp` / `strlen` family | done | including both memmove overlap directions |
| `kmalloc` / `kfree` free-list allocator | done | first-fit, splits, coalesces both ways |
| `printk` formatting | done | width/padding/length modifiers; unsupported specifiers echoed, not dropped |
| indirect-call dispatch (`crt_callN`) | planned | needs the emitter |

`crt` is 104 functions and compiles standalone. Its output was diffed against
the equivalent C compiled with `cc -fno-builtin` — byte for byte identical
across 44 cases. Known gaps, all documented in `crt/README.md`: `%c` of NUL
(an `id` string cannot carry an embedded NUL), precision/`*`-width/`+`/`#`
flags, and floating-point conversions.

## `c2id` — the C→`id` compiler, in `id`

| stage | state | evidence |
| --- | --- | --- |
| project layout scheme (rule of 3) | done | 0 → `b/0/0/0/0/0/f0.id`, 2186 → `b/2/2/2/2/2/f2.id` |
| AST node store | done | builds and reads nodes under `idc.py` |
| statement parser | done | all 15 C statement forms, trees in `c2id/parse/code/st/README.md` |
| frame layout | done | offsets and size identical to `cc`'s for the same struct |
| C lexer | done | 520,450 tokens from `lib/bsearch.i` in 0.054 s; line/file tracking matches the real sources |
| declaration / type parser | done | 212 functions; struct layout identical to C across 14 records incl. `iphdr`/`tcphdr` bitfields |
| expression parser | done | precedence and associativity verified by fully-parenthesised printing |
| CFG construction | planned | spec in `docs/EMITTER.md` §2 |
| `id` emitter (block functions + dispatch tree) | planned | target form proven in `tests/lowering/` |
| indirect-call dispatch | planned | spec in `docs/EMITTER.md` §4 |

**The front end works end to end** — when built alone. Lexer → expression
parser → statement parser compile and run as one program, and real C
statements parse correctly:

```
if (p->len > 0) { total += arr[i].n; } else { return -1; }
for (i = 0; i < n; i++) { s = f(a, b); }
switch (k) { case 3: goto done; default: break; }
```

`c2id` now compiles as one program — lexer, AST store, type and declarator
parser, expression parser, statement parser, frame layout and the project
layout scheme all coexist. Casts, declarations, function-pointer declarators
and kernel-shaped pointer chains all parse with no diagnostics.

**But `tests/run.sh` case `frontend-integration` currently fails**, because
that harness builds the front end alongside `idstd` (implicitly imported —
see `docs/ID_CHEATSHEET.md`), and `c2id`'s own `lset` collides with the
library's. See "Known gaps in what is built" below for the exact evidence.

**Nothing is emitted yet.** The back end (`emit/cfg/`, `emit/gen/`) is fully
specified in `docs/EMITTER.md` but not written, so no C compiles to `id` and
the differential cases in `tests/c/` still report SKIP. See
`docs/CONTINUING.md`.

## Known gaps in what is built

Recorded here rather than left to be discovered:

* **`c2id`'s own `lset` collides with `idstd`'s.** `tests/frontend/build.sh`
  builds `c2id/lex` and `c2id/parse` together with the harness in
  `tests/frontend/harn`, and `idstd` is implicitly imported into every `id`
  project. `c2id/lex/drv/st/store.id:9` defines `lset(int[] xs, int i, int
  v)` — needed because `(import xs)[i] = v` silently does nothing (see
  `c2id/NOTES.md` §6) — and `../idstd/core/data/lst/lst.id:21` defines the
  same name for the same reason. The build fails with:
  ```
  error: function 'lset' already defined at
  /home/preland/git/id_development/c2id/build/frontend/src/c2id/lex/drv/st/store.id:9
  ```
  This is the exact state `id_development`'s `tests/idstd_expect.txt` calls
  `broken`: a private copy of a name the standard library already defines,
  same class of problem the compiler's own `lex`/`parse` stages had before
  they were switched to call idstd's `lset` instead of defining their own
  (see that file). The fix here is the same one, not attempted in this pass.
  Measured this session: `tools/check.sh` → 2 passed, 1 failed
  (`frontend-integration`), 0 skipped; `tests/run.sh` → 2 passed, 1 failed,
  7 skipped.
* **Large integer literals truncate in the C parser.** `c2id`'s AST stores a
  numeric literal's value in an `int[]` field, and `id`'s `int` is 32 bits, so
  `0x0123456789abcdefUL` reaches the emitter as `-1985229329`. Found by
  differential testing during expression emission and confirmed against a
  lexer+parser-only harness, so it is a front-end defect, not an emitter one.
  This blocks any C source that relies on wide constant masks. The fix is to
  read the literal's spelling — already preserved on the node — rather than
  its truncated value.
* **`<<` width follows the operand type**, in `id` exactly as in C: `1 << 40`
  is 0; `w << 40` with a `word` `w` is correct. Not a defect, but it means the
  emitter must widen constant left operands whenever the source relies on a
  wide shift. Documented in `docs/ID_EXTENSIONS.md`.
* **Constant-expression precedence is 2 levels, not C's 12.** `1 | 1 << 3`
  evaluates to 8, not 9. This is the one place the front end can produce a
  silently wrong answer, and it must be fixed before any translation depends
  on an array bound computed from shifted constants.
* **K&R parameter lists** (`int f(a,b) int a,b;`) mis-parse. Not reachable
  with one token of lookahead.
* **`__attribute__((packed))` / `((aligned(N)))`** are parsed and skipped
  correctly but do not affect layout. Structures that rely on `packed` will
  have wrong offsets — this needs closing before any on-the-wire structure is
  ported.
* **Octal literals read as decimal** in the type module's constant evaluator
  (`010` → 10).
* **`%c` of NUL emits nothing** in `crt`. An `id` string cannot carry an
  embedded NUL; unfixable without a language change.

## Known gaps that will not close soon

* **Inline assembly.** Untranslatable by construction. Handled by matching
  against a table of hand-written `crt` intrinsics; anything unmatched is
  reported as a counted gap rather than silently mistranslated.
* **`volatile` / MMIO.** Has no meaning in the flat-store model without a
  device simulation layer.
* **Concurrency.** Translated code is single-threaded; locking primitives
  become no-ops, which is correct for a single-threaded execution and wrong
  for anything else. Flagged rather than pretended away.
