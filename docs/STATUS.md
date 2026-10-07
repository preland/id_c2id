# Status ledger

The rule for this file: a row says "works" only if a command in `tests/` runs
and passes. Everything else is "planned" or "in progress", regardless of how
much code exists.

Last updated: 2026-10-05. Counts that change are not kept here: each row names
the command that prints them.

## Infrastructure

| item | state | evidence |
| --- | --- | --- |
| Differential test harness | works for the programs in `tests/c/` (stdout, stderr and exit status each compared; `NAME.in` is a program's stdin, `NAME.refuse` the lines a refusal must contain); **`frontend-integration` fails** | `tests/run.sh` — C oracle vs id, byte-for-byte; the front-end harness copies `front/lex` and `front/parse` alone, and `front/parse/.../decl_init.id` calls `complete_arr`, which lives in `back/link/ext/decl/size.id`, so it no longer builds (a `front` → `back` dependency the restructure introduced) |
| Block-function lowering | **proven** | `tests/lowering/` — compiles under `bin/idc`, computes 55 |
| c2id driver (stream → project tree) | works | `tools/c2id.sh`, which builds `c2id` with `bin/idc` under the default test requirement |
| Pre-commit entry point | **1 known failure** (`tests`, from `frontend-integration` above) | `tools/check.sh`; `C2ID_BIN` names the translator binary for `tools/c2id.sh`, and `C2ID_SH` names the driver script for `tests/run.sh` |
| `id` toolchain probe | works | `tools/idprobe.sh` — 0 / 1 / 2 for usable / too old / absent |
| CI | **unproven** | `.github/workflows/ci.yml` — clean under `actionlint` where checked; every command in it passes locally; never yet run on a GitHub runner |

## The compiler itself (`../idc`)

`bin/idc` is the compiler, and the only one: `idc.py` is deleted, and stage 0 is
the checked-in C in `../idc/bootstrap/`.

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

Every extension there is implemented by `idc/bin/idc` on all three targets.
To see that it still is, run `idc/tests/conform.sh` in the umbrella repo: its
`int/` and `store/` areas cover hex and binary literals, the bitwise operators,
`word`, the flat store and the unsigned builtins, each on `c`, `llvm` and `wasm`.

## `crt` — C-semantics runtime, in `id`

| item | state | evidence |
| --- | --- | --- |
| integer width truncation / sign extension | done | boundaries match C exactly |
| `memcpy` / `memmove` / `memset` / `memcmp` / `strlen` family | done | including both memmove overlap directions |
| `kmalloc` / `kfree` free-list allocator | done | first-fit, splits, coalesces both ways |
| `printk` formatting | done | width/padding/length modifiers; unsupported specifiers echoed, not dropped |
| indirect-call dispatch (`crt_callN`) | planned | needs the emitter |

`crt` is 104 functions and compiles standalone under **both** compilers, which
it did not until 2026-08-29: it had 77 violations of two rules `bin/idc`
enforces and `idc.py` does not (`../docs/GAPS.md` B6/B7), because nothing had
ever built it with `bin/idc`.

This row used to claim its output "was diffed against the equivalent C compiled
with `cc -fno-builtin` — byte for byte identical across 44 cases". **No such
command was in this repository**, and none ever had been; the claim broke this
file's own rule. `tests/crt/run.sh` is what backs the row now — a smoke test,
not 44 cases: printf's radix, sign, width, flags and length modifiers, then
`strlen`, `strcmp` and `memset`, against `tests/crt/twin.c`. The allocator's
addresses are not compared with C's (C's malloc is not this allocator); its
invariants are.

Known gaps, all documented in `crt/README.md`: `%c` of NUL (an `id` string
cannot carry an embedded NUL), precision/`*`-width/`+`/`#` flags, and
floating-point conversions.

## `c2id` — the C→`id` compiler, in `id`

The source is `c2id/`: `front/` (lexer, type and expression and statement
parsers, constant folding), `mid/` (the pass that gives every local a frame
slot and renames shadowed ones, reachability, and the lowering of `&&`, `||`,
`?:` and side effects into blocks) and `back/` (CFG construction, block text,
function-pointer dispatch, and the link step that emits globals, string
literals, initializers, call wrappers and `main`). `rt/` and `crt/` are what a
translated program links against.

| stage | state | evidence |
| --- | --- | --- |
| translator builds under the default test requirement | works | `idc c2id` exits 0: every function has two cases, and they pass |
| C → id for integer C, pointers, structs, function pointers, initializers, string literals | works | the corpus below: the programs there match `gcc -O2` |
| floating point | **absent, refused** | `double` has no model; any use (a type, a literal, a call that returns one, `%f`) is a translate-time refusal with `file:line`; see "Floating point" below; `tests/c/915_refuse_float.c` |
| refusals | works | what c2id cannot translate correctly it refuses with `file:line` and the construct, writes nothing and exits 1: a struct passed or returned by value, bitfields, compound literals, variadic definitions, `fopen` for writing, `setjmp`/`longjmp`, termios, floating point, printf-family conversions the runtime does not implement, and every called function nothing provides (one diagnostic lists them all); `tests/c/9*.c`, `c2id/NOTES.md` §7 |
| `static` locals | works | a `static` local is a global of its own (`mid/pass/frame/static.id`); `tests/c/100_static.c` |
| libc the runtime provides | works for the names in `rt_names` (`c2id/conf.id`) | strtol (and, over it, atoi, atol, strtoul, strtoll), the str* family, `qsort` and `bsearch` (C in `include/stdlib.h`, since they call back into the program), stdio over real stdin/stdout/stderr (`getchar`, `fgetc`, `ungetc`, `fgets`, `fread`, `fwrite`, `fputc`, `fflush`), `sscanf` over integers and strings, `time`, `clock`, `read`/`write`/`close` on 0, 1 and 2; `tests/c/080` to `165` |
| block-function lowering and dispatch | works | `tests/lowering/`, and every corpus program |
| indirect-call dispatch | works | `tests/c/060_funcptr.c`, and the function-pointer tables in the corpus |
| speed | **75x to 183x slower than `gcc -O2`** | `docs/INTEROP.md`, "Performance against gcc -O2"; the optimizer level changes nothing measurable |

The parsers handle real C:

```
if (p->len > 0) { total += arr[i].n; } else { return -1; }
for (i = 0; i < n; i++) { s = f(a, b); }
switch (k) { case 3: goto done; default: break; }
```

### The translator's own tests

`idc c2id` is the evidence. A function without two cases is a build error, so
there is no untested function to list; `idc c2id --list-untested` must print
nothing. The cases reach the back end through real C: a case names a setup that
lexes the case's `stdin` and loads it into the translator's tables
(`mid/pass/fixture/tu.id`, `tu_load`), then calls one function and compares what
it printed (`then prints`) or left behind (`then (import pend)`, `bkind`, `bsa`,
`lvnames`, ...). The lowering cases start from `low_fix`
(`mid/pass/fixture/low.id`), which builds a few expression nodes by hand and one
open block.

Three things this suite found when it first ran over the `front/mid/back`
restructure, which had been written against an older idstd:

* `str_slice` is `(string, start, count)` in the current idstd and was called
  as `(start, end)` in three places (`unquote`, and the escape decoders
  `lit_esc` and `lit_besc`). A string literal with a backslash escape, or a
  character constant, decoded wrongly. Fixed at the three call sites.
* Four cases in `mid/pass/fixture` named node numbers that had shifted by six
  when the registries began to allocate their own nodes first; they failed
  before any new case ran. The numbers are corrected.
* Two functions had logic identical to idstd's: `cur_blk` to a reader of
  `idstd_ec_c[0]`, and `ty_uint` to one of `idstd_ec_c[1]`. `cur_blk` now reads
  through idstd's `lget`; `ty_uint` is gone and `num_uty` reads
  `(import stdty)[1]` itself, where its own two cases cover the index.

Mutation check, by hand: about 35 of the functions with new cases were broken
(a flipped comparison, an off-by-one, a swapped edge), in batches, and the break
turned a case of that function red each time, once two functions whose cases
could not tell the break apart (`cv_cmp` for `&&`, `bind_one` for the type index)
were given the input that can. A passing case is not evidence on its own, and a
failing callee hides its callers' cases ("not run"), so mutations were grouped to
keep one from masking another.

### Corpus

Each program is preprocessed, translated, built with `idc` and run; stdout and
exit status are compared with the same source built by `gcc -O2`. The programs
are inih, the libmd digests (md5, sha1, sha2), xxhash, lz4, miniz's inflate,
cJSON, two micro programs and the seven in `tests/c/`; they are not in this
repository (the driver and the sources were kept in an earlier wave's scratch
directory), and the table of results is in the umbrella's `docs/INTEROP.md`
under "Translating C to id". The last run: every program matches except cJSON,
which c2id refuses at translate time because it uses floating point (it used to
stop in `idc`, at link time, on the first missing function).

### Floating point

`double` is the gap that stops cJSON, and it is a missing model rather than a
bug; until it exists every use is refused (`mid/check/flt.id`). Where the work goes, once idc has a float bit-cast builtin (one that turns
a 64-bit pattern into a `float` and back):

* `back/emit/gen/txt/ex/a/w/cv.id`: `load_txt` emits `peek<bits>(addr)` and
  `store_txt` emits `poke<bits>(addr, v)`; for a `double` they become
  `bits_to_float(peek64(addr))` and `poke64(addr, float_to_bits(v))`, and a
  C `float` goes through the 32-bit pair. The width table beside it
  (`.../wid/width.id`, `load_wid`, `wid_bits`) needs the float rows.
* `back/link/fn/wrap/entry.id` and `back/link/ext/call/args.id`: a call passes
  `word` arguments through frame slots with `poke64(fp + 8 + 8 * i, aN)` and
  reads the result with `peek64(fp)`. A `double` parameter or result must be
  bit-cast on both sides, so the wrapper `c_NAME` has to know which parameters
  are floating.
* `front/parse/code/ex/p/pf/lit/num/n1.id` and `front/parse/code/ex/s/sub/s3.id`:
  a floating literal has no value yet (`is_float_lit` keeps the spelling).
  Its value is emitted as a float literal in
  `back/emit/gen/txt/ex/d/y/lit/atoms.id`.
* the operators and conversions in `back/emit/gen/txt/ex/` (`conv_txt` in
  `cv.id`): `+ - * /` and comparisons on floating operands run on `float`
  variables, and int/float casts are conversions, not bit-casts.
* `rt/ext/stdio/out.id` and `rt/ext/builtin/`: `%f`/`%g`, `strtod`, `fabs`,
  `sscanf("%lf")` over the bit-cast.

## Known gaps in what is built

Recorded here rather than left to be discovered:

* **`exit()` and `abort()` do not end the block they are called in.** They record
  the status (`rt/libc/exit.id`) and the dispatch loop stops at the next block
  boundary, so a statement after the call in the same basic block still runs,
  and so does the rest of a caller's block when the call was in a callee.
  Nothing refuses this yet; `tests/c/165_abort.c` has nothing after the `abort()`
  for that reason.
* **printf formats held in a variable are not checked.** A literal format with a
  conversion the runtime does not implement is refused; one built at run time
  echoes the conversion instead (`crt/README.md`).
* **`strtol` wraps instead of clamping** on a value out of range and does not set
  `errno`; `sscanf` has no `%[...]`, no float conversions and ignores a width on
  the integer conversions.
* **Four names this repository defined and `idstd` already owned** — fixed.
  `lset` and `sset` were byte-identical to the library's, `max_int` was
  `fx_max` with its locals spelled differently, and `str_join` merely shared a
  name with an unrelated library function, so it became `str_glue`. Only the
  first was ever reported, because the compiler stops at the first collision;
  the rest were found by fingerprinting every function in both trees the way
  the uniqueness rule does. Two more arrived with the `front/mid/back`
  restructure (`cur_blk` and `ty_uint`, against `idstd_ec_t_status` and
  `idstd_ec_t_extra`), and `crt/`'s `wrap32` was removed for the same reason: it
  was `sx32`.
* **Large integer literals truncate in the C parser's `int[]` field** — worked
  around in the back end. `c2id`'s AST stores a numeric literal's value in an
  `int[]` field, and `id`'s `int` is 32 bits, so `0x0123456789abcdefUL` reaches
  the emitter as `-1985229329`. `back/link/data/num/spell.id` (`num_spell`) reads
  the spelling the node keeps instead, for hex and non-leading-zero decimal
  literals; an octal literal still goes through the truncated value.
* **`<<` width follows the operand type**, in `id` exactly as in C: `1 << 40`
  is 0; `w << 40` with a `word` `w` is correct. Not a defect, but it means the
  emitter must widen constant left operands whenever the source relies on a
  wide shift. Documented in `docs/ID_EXTENSIONS.md`.
* **Constant-expression precedence** — fixed. Constants are parsed by the
  ordinary expression parser and folded over the tree (`front/fold/`), so
  `1 | 1 << 3` is 9; `const_eval`'s cases include it.
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
