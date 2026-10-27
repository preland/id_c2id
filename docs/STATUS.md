# Status ledger

The rule for this file: a row says "works" only if a command in `tests/` runs
and passes. Everything else is "planned" or "in progress", regardless of how
much code exists.

Last updated: 2026-10-05. Counts that change are not kept here: each row names
the command that prints them.

## Infrastructure

| item | state | evidence |
| --- | --- | --- |
| Differential test harness | works for the programs in `tests/c/` (stdout, stderr and exit status each compared; `NAME.in` is a program's stdin, `NAME.refuse` the lines a refusal must contain) | `tests/run.sh` — C oracle vs id, byte-for-byte; `frontend-integration` builds the whole of c2id with its driver swapped for `tests/frontend/harn/main.id` (the parser reads the diagnostics, the type sizes and the folder reads `back/`, so it no longer builds from `front/lex` and `front/parse` alone, which is why it failed) |
| Block-function lowering | **proven** | `tests/lowering/` — compiles under `bin/idc`, computes 55 |
| c2id driver (stream → project tree) | works | `tools/c2id.sh`, which builds `c2id` with `bin/idc` under the default test requirement |
| Pre-commit entry point | its one known failure (`frontend-integration`) is fixed; the whole of `tools/check.sh` was not run after that fix | `tools/check.sh`; `C2ID_BIN` names the translator binary for `tools/c2id.sh`, and `C2ID_SH` names the driver script for `tests/run.sh` |
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
| floating point | works for `float`, `double` and `long double` | a double is the 64 bits of its binary64 in a `word`, a float the same word holding a binary32 value (rounded after every operation); `crt/flt` does the arithmetic through the float bit-cast builtins, `crt/fmt/dec` and `crt/fmt/flt` print `%f %e %g` from the exact digits, `rt/ext/math` is libm over idstd's `flt_`, `rt/ext/num/dbl*.id` is `strtod`; `tests/c/174_float_arith.c` to `179_libm.c`, and the four float bench programs (fasta, mandel, nbody, spectral) match `gcc -O2`; a `long double` is 16 bytes handled like a struct (its value is the address of its bytes) and `crt/ldbl` is the x87 80-bit extended format in `id` (+ - * / rounded once to nearest even at 64 bits, conversions, comparisons, NaN/infinity/denormal rules, `%Lf %Le %Lg`, `strtold`, literals), proved against the compiler's x87 by `tests/c/227_ld_basic.c` to `231_ld_sweep_d.c` (the sweeps hash millions of random operand pairs); `l` libm functions, `%La` and `sscanf` `%Lf` are not provided |
| refusals | works | what c2id cannot translate correctly it refuses with `file:line` and the construct, writes nothing and exits 1 (a syntax error among them, which also ends the parse): a struct passed through the `...` of a variadic call, inline `asm` that does something (an instruction, an output operand, `asm goto`; one at the top of a unit is refused when the program reaches any function, `front/../unit/gasm.id`, `mid/check/gasm.id`), a bit-field of a 128-bit type (refused where a reached function uses its record), a constant expression whose value is outside `[-2^31, 2^32-1]` (`cw_wide`), a variable bound to a register, a non-constant array size (a variable length array), the `constructor`, `destructor` and `alias` attributes, `_Generic`, `typeof(expression)` where its type is not known (a name that is a local or a parameter outside a local declaration, a cast or a `sizeof`), a `cleanup` variable declared anywhere but a declaration statement of a block, the address of a label and a computed `goto`, a compound literal or statement expression inside a brace initializer, a `case` or designator range of more than 4096 values, an `fopen` mode that is not r, w or a followed by b and `+` alone, termios, signals, a 128-bit integer converted to or from a floating-point type or passed through `...`, `packed` on a variable (not a member), a zero-width or over-long bit-field in a packed record, a named address space (`__seg_gs`); inline asm, register variables, `_Generic`, the bad `typeof`, a non-constant array size, a named address space and a `cleanup` of something other than a plain name are refused only when a function the program reaches holds them (a declaration that defines no function poisons the variables and types it declares instead, and a reached function that uses one is refused: `front/diag/defer*.id`, `seg*.id`), the address of a runtime function that has no wrapper (printf and the other variadic ones) or of a function defined with `...`, printf-family conversions the runtime does not implement, and every called function nothing provides (one diagnostic lists them all); `tests/c/9*.c`, `c2id/NOTES.md` §7 |
| `packed`, `aligned`, packed enums | works | `struct __attribute__((packed)) S`, `struct S {...} __attribute__((packed, aligned(N)))`, a packed union, a member written `T m __attribute__((packed))`, and a packed enum (the smallest integer type that holds its enumerators) are laid out as gcc lays them out (`front/parse/ty/dcl/r/pk/`, `e/pk/`); a bit-field in a packed record starts at the very next bit and is read through the bytes it spans (`back/emit/gen/txt/ex/a/bf/span.id`); sizes, alignments and offsets compared with gcc in `tests/c/263` to `265` and `267`; `aligned(N)` on a member or a typedef is still ignored |
| 128-bit integers | works | `__int128`, `unsigned __int128`, `__int128_t` and `__uint128_t` are 16-byte aggregates like a long double, with `crt/i128` doing `+ - * / %`, the bitwise operators, shifts, the comparisons, truth tests and the conversions to and from the integer types; a million random operand pairs per group hashed against gcc's own `__int128`: `tests/c/277_i128_sweep_arith` (+ - * & \| ^ ~ negation, the twelve comparisons, `!` `&&` `\|\|` `?:`), `_shift` (every count 0 to 127 for `<<`, logical and arithmetic `>>`), `_div` (signed and unsigned `/` and `%`) and `_mix` (the operators against `int`, `long`, `short`, `char`, `_Bool`, compound assignment, `++`), plus `276` (named values), `278` (globals, arrays, structs, calls, function pointers, recursion) and `279`; a division by zero answers 0 and the dividend; not provided: conversions to and from `float` and `double`, `...` |
| `__attribute__((cleanup(fn)))` | works | on a local declared in a block: the rest of the block becomes a scope of its own (`front/parse/code/sb/cl/`) that calls `fn(&var)` at its end and before every `return` (the value is computed first, into a variable of its own), `break` or `continue` that leaves it and `goto` to a label outside it; two cleanups in one declaration run in the reverse order; `tests/c/274` |
| `typeof(expression)` | works where the type is known when it is read | a name that is a global (and not also a parameter or local of the declaration being read), a cast, `sizeof`, a member of a cast null pointer: in file scope, a parameter list, a struct member or a function body (`front/parse/ty/spec/sp/tag/tof/`); `tests/c/266`; a local or parameter inside a local declaration, cast or `sizeof` is the frame pass's as before |
| kernel units (`kall/*.i`) | partly | a unit with no `main` starts from its external functions, those with a `used` attribute and the initializers of its external variables (a `static` or `inline` definition is dropped unless something reached names it); `C2ID_UNSIGNED_CHAR=1` makes plain `char` unsigned for code built with `-funsigned-char` (the kernel, whose preprocessed output does not say so); a unit with no `main` gets a `main` that runs the global initializers and answers 0 (`back/link/data/boot/call.id`); a call through a pointer with no function of that arity defined calls `abort` (`back/ind/note.id`) |
| statement expressions `({ ... })` | works | a `stmtx` node lowered into the graph, its value in a frame temporary; `tests/c/193`, `194` |
| block scope of typedef names, tags and enumerators | works | a name declared in a block ends with it, and the innermost wins; `tests/c/192` |
| variadic definitions, `va_list`, `vsnprintf` and friends | works | `docs/EMITTER.md` "Variadic definitions"; `tests/c/196`, `197`, `198`, `208` |
| bit-fields | works | shift and mask on the declared type's unit, layout as gcc's on x86-64 (sizes compared in `tests/c/210`); an enum-typed field is unsigned unless an enumerator is negative, as in gcc (`tests/c/260`) |
| `setjmp`/`longjmp`, `sigsetjmp`/`siglongjmp` | works | `docs/EMITTER.md` "setjmp and longjmp"; `tests/c/213`, `215`, `216` |
| `switch` subject with side effects | works | evaluated once; `tests/c/214` (it was evaluated once per case test) |
| `static` locals | works | a `static` local is a global of its own (`mid/pass/frame/static.id`); `tests/c/100_static.c` |
| structs and unions passed and returned by value | works | the caller passes the address of a struct argument and the wrapper copies it into a frame slot of the struct's size; a function returning one takes a hidden first parameter, a pointer to a frame temporary the frame pass gives each call (`docs/EMITTER.md`, "Struct values"); direct and function-pointer calls, `f().x`, `f(g(), g())`, initialisers (also brace lists), assignment, conditions and loops, recursion, callees that exit; `tests/c/057`, `058`, `059`, `189` to `191`. Refused: a struct passed through `...` (`tests/c/921`) |
| files written | works | `fopen` with `w`, `a`, `r+`, `w+`, `a+` (and `b`), then `fwrite`, `fputs`, `fputc`, `fprintf`, `fflush`, `fclose`, `fseek`/`ftell`, `remove`, `rename`, `tmpfile`; a written file is held whole in the heap and saved to its path by `fflush`, `fclose` and when the program ends, so another reader sees the bytes only after one of those (`rt/ext/stdio/file/w*.id`, `docs/EMITTER.md` "Files written"); `tests/c/224_file_write.c` writes, appends, overwrites and reads files back, and `933` refuses a mode such as `wx`. Not done: `fseek` past the end of a written file (answers -1), `abort` saves the written files where C's does not |
| GCC atomic and `__sync` builtins | works | single-threaded, so each is the plain operation with the builtin's answer: `__atomic_load_n`, `_store_n`, `_exchange_n`, `_compare_exchange_n`, the `fetch_OP` and `OP_fetch` forms, `test_and_set`, `clear`, the fences, the generic forms (`__atomic_load`, `_store`, `_exchange`, `_compare_exchange`) and the `__sync_*` family, on objects of 1, 2, 4 and 8 bytes (any size for the generic copies); a call is rewritten in the frame pass (`mid/pass/frame/atom/`, tables in `conf.id` `atom_*`) because the size and the type of the answer come from the type of the first argument; `tests/c/225_atomic_builtins.c`. C11 `<stdatomic.h>` is not provided (`932`) |
| address of a runtime function | works for the functions the headers wrap | the headers define `__c2id_w_NAME` for the string, ctype, math, stdlib, stdio, unistd and time functions the runtime provides, and a name used as a value (`{ malloc, free }`, `qsort(.., strcmp)`, a table of `strlen`) is rewritten to its wrapper, which is reached and so has a place in the function table (`mid/check/wrap.id`); `tests/c/226_libc_fn_addr.c`; the variadic ones (printf, sprintf, sscanf, ...) have no wrapper and are refused (`924`, `931`) |
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
under "Translating C to id". The last run: every program matches, cJSON among them.
cJSON's `global_hooks = { malloc, free, realloc }` takes the address of runtime
functions, which used to compile to a table of zeros and recurse without end in
`cJSON_New_Item` (a silent miscompile, then a translate-time refusal); the headers now
define a wrapper for each such function and the address is the wrapper's
(`mid/check/wrap.id`, `tests/c/226`), so the original file translates and parses
JSON. Before that, `cJSON_Parse` used to fail on every string and number. The cause: a struct assigned
or initialised from another struct (`buffer.hooks = global_hooks;`, `B c = b;`,
`t->slots[h] = old[i];`) was stored as the 8-byte address of the source, not copied.
`store_raw` (`back/emit/gen/txt/ex/a/w/cv.id`) now emits a `c_memcpy` of the type's
size for a record; `tests/c/051_struct_copy.c` covers nested structs, arrays of
structs and copies through pointers, and `tests/c/054_struct_table_grow.c` is the
hash-table growth that made `bench/ref/c/knuc.c` print a wrong count on inputs of
about 6 KB and more (the table grows past 512 distinct keys). A struct passed or
returned by value is translated (below).

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

What is not done: hexadecimal floating literals and
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
* **Constant-expression types** — fixed. The folder evaluates in 64-bit words with C's
  types (`front/fold/calc/`: literal suffixes, the usual arithmetic conversions, unsigned
  division, comparison and shift, casts that truncate and sign-extend), so `-1 < 0u` is 0,
  `(unsigned char)300` is 44 and `~0UL >> 60` is 15; `tests/c/280`, `281`. What it hands
  its callers is an int, so a value outside `[-2^31, 2^32-1]` is refused (an `enum` value,
  an array bound), and an enumerator in `(2^31-1, 2^32-1]` is still stored as the int of
  the same bits, as before.
* **K&R parameter lists** (`int f(a,b) int a,b;`) mis-parse. Not reachable
  with one token of lookahead.
* **`__attribute__((aligned(N)))`** is read on a struct, union (`aligned(N)` with a literal N, or alone for 16) and ignored on anything else: a member or typedef declared `aligned(16)` is laid out at its natural alignment, so the `sizeof` of a struct holding one differs from gcc's.
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
