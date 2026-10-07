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
| floating point | works for `float` and `double`; `long double` is refused | a double is the 64 bits of its binary64 in a `word`, a float the same word holding a binary32 value (rounded after every operation); `crt/flt` does the arithmetic through the float bit-cast builtins, `crt/fmt/dec` and `crt/fmt/flt` print `%f %e %g` from the exact digits, `rt/ext/math` is libm over idstd's `flt_`, `rt/ext/num/dbl*.id` is `strtod`; `tests/c/174_float_arith.c` to `179_libm.c`, and the four float bench programs (fasta, mandel, nbody, spectral) match `gcc -O2`; `tests/c/915_refuse_float.c` is the `long double` refusal |
| refusals | works | what c2id cannot translate correctly it refuses with `file:line` and the construct, writes nothing and exits 1: a struct passed or returned by value, bitfields, compound literals, variadic definitions, `fopen` for writing, `setjmp`/`longjmp`, termios, `long double`, the address of a runtime function (`{ malloc, free }` as a hooks table), printf-family conversions the runtime does not implement, and every called function nothing provides (one diagnostic lists them all); `tests/c/9*.c`, `c2id/NOTES.md` §7 |
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
under "Translating C to id". The last run: every program matches except cJSON.
cJSON now translates past its floating point (parse_number's strtod, print_number's
`sprintf("%1.15g")`), and stops at two things that are not floating point. Its
`global_hooks = { malloc, free, realloc }` takes the address of runtime functions,
which used to compile to a table of zeros and recurse without end in `cJSON_New_Item`
(a silent miscompile; it is now a translate-time refusal, `mid/check/rtval.id`). With
the table written as wrapper functions (the file's own MSVC branch) it builds and runs,
but `cJSON_Parse` used to fail on every string and number. The cause: a struct assigned
or initialised from another struct (`buffer.hooks = global_hooks;`, `B c = b;`,
`t->slots[h] = old[i];`) was stored as the 8-byte address of the source, not copied.
`store_raw` (`back/emit/gen/txt/ex/a/w/cv.id`) now emits a `c_memcpy` of the type's
size for a record; `tests/c/051_struct_copy.c` covers nested structs, arrays of
structs and copies through pointers, and `tests/c/054_struct_table_grow.c` is the
hash-table growth that made `bench/ref/c/knuc.c` print a wrong count on inputs of
about 6 KB and more (the table grows past 512 distinct keys). A struct passed or
returned by value is still refused.

A program now receives its arguments: the generated `main` calls `rt_args`
(`rt/args.id`), which copies `argv` into the store, and passes `argc, argv` on to
`c_main` as far as the C `main` declares parameters (`tests/c/052_argv.c`, with its
arguments in `052_argv.args`).

### Floating point

A C `double` is the 64 bits of its IEEE binary64 in a `word`; a `float` is the
same word holding a value that binary32 represents, so both are one thing in an
expression and the type only decides what happens at a load, a store and the end
of an operation. The pieces, each with cases:

* `back/emit/gen/txt/ty/fl/`: `ty_fkind` (0, float 1, double 2), the usual
  conversions (`usual_ty`, `fl_pair`), a literal's type and text (`fl_lit_txt`:
  `.5`, `5.`, `1.f` are rewritten to the form id reads, and an `f` literal is
  rounded to binary32), binary operators on floating operands (`fl_bin_txt`:
  both operands converted to the common type, the result rounded to binary32 for
  `float`), `-` (a sign flip, so `-0.0` stays), `!`, the truth test of a branch
  (`cond_val`: -0.0 is false, a NaN is true), and the conversions (`fl_cvt`:
  int to double exactly, double to int truncating toward zero, double to float
  rounding to nearest even).
* Conversions are made explicit as `cast` nodes where a value is stored,
  initialises, is passed or is returned (`fl_coerce`; `assign_store`,
  `comp_store`, `init_val`, `fl_args` for a call's arguments, `retval_txt` for a
  `return`), so `double d = 3;`, `sqrt(2)`, `int i = 2.9;` and `return 0;` in a
  `double` function all convert; a float parameter is stored as binary32 by the
  call wrapper (`wrap_arg`).
* `crt/flt/`: `cfl_add`, `cfl_sub`, `cfl_mul`, `cfl_div`, the six comparisons
  (false for a NaN except `!=`), `cfl_neg`, `cfl_rnd32`, `cfl_ld32`, `cfl_st32`
  and the integer conversions, over the four float bit-cast builtins.
* `crt/fmt/dec/`, `crt/fmt/flt/`: `%f %F %e %E %g %G` from the exact decimal
  digits of the double (the mantissa times a power of 2 or 5 as a big decimal),
  rounded half to even as glibc does, with the flags `- 0 + space #`, width and
  precision, `inf`/`nan`/`-nan`. One glibc quirk is copied on purpose: `%#g` of a
  value whose rounding carries into the next power of ten has no trailing zeros
  (`999999.5` prints `1.e+06`).
* `rt/ext/math/`: libm over idstd's `flt_` natives (sqrt, cbrt, hypot, the
  trigonometric, hyperbolic, exponential and logarithmic functions, pow, fmod,
  atan2, floor, ceil, round, trunc, fabs, copysign, fmin, fmax, ldexp, isnan,
  isinf, isfinite, and `NAN`/`INFINITY`); `rt/ext/num/dbl*.id`: `strtod` (and
  `atof` over it) and the `%f %e %g` conversions of `sscanf`, through idstd's
  `flt_parse` (decimal only; `inf` and `nan` are made here).

What is not done: `long double` (refused), hexadecimal floating literals and
`%a`, `frexp`/`modf` (they write through a pointer), the single-precision libm
names (`sqrtf` and the rest; refused as unprovided), a `float` literal's double
rounding (see `fl_lit_txt`), and a conversion of an out-of-range double to an
integer, which is undefined in C and gives whatever the hardware does.

## Known gaps in what is built

Recorded here rather than left to be discovered:

* **`exit()` and `abort()` end the block they are called in.** A call to `exit`,
  `abort`, `_Exit` or a function declared `_Noreturn` or
  `__attribute__((noreturn))` ends its basic block (`back/emit/cfg/w/j/plain.id`,
  the names from `front/diag/noret/`), and a statement that calls a function which
  may reach one (any function that calls one or calls through a pointer, closed
  over the program: `mx_close`) is followed by a block boundary, where the runner
  sees the exit status; so the statements after such a call never run
  (`tests/c/166` to `173`, `184`). A call of that kind inside a larger expression,
  as in `printf("%d", die())`, is stored into a temporary and the block is cut
  after it (`mid/lower/val/call.id`, `tests/c/187`, `188`), except a call through
  a name nothing declares (a runtime builtin), which is not. `_Exit` flushes the
  output buffer, which C's does not.
* **Stack depth.** The stack grows in chunks and traps with `stack overflow` past
  64 MiB of them (`rt/stackcap.id`, `tests/c/185`), but a program with small
  frames reaches the host stack limit first (about 170000 nested calls under an
  8 MiB limit) and dies with SIGSEGV and no message.
* **printf formats held in a variable are not checked.** A literal format with a
  conversion the runtime does not implement is refused; one built at run time
  echoes the conversion instead (`crt/README.md`).
* **`strtol` wraps instead of clamping** on a value out of range and does not set
  `errno`; `sscanf` has no `%[...]` or `%a`, and ignores a width on the float and
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
