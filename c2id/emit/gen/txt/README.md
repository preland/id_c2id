# `emit/gen/txt` — expression and statement text

The stage that turns one C expression or one straight-line C statement into
the `id` source text that computes it. `docs/EMITTER.md` §3 is the
specification; `tests/lowering/` is what the output has to look like.

Nothing here knows about basic blocks, and nothing here writes a file. It
produces one string per node, and the CFG builder and the project writer
decide where the strings go.

## The one invariant

**Every emitted expression is a `word` holding the C value in one canonical
form**: a signed C type narrower than 64 bits arrives sign-extended, an
unsigned one zero-extended, an 8-byte type as its 64 raw bits.

That single rule is what lets every operator be chosen from the type alone.
There is never a question of what state a subexpression left its value in, so
`sx32(peek32(fp + 8)) / sx32(peek32(fp + 12))` is correct without either side
knowing anything about the other, and the same `/` would be wrong only if the
type said unsigned — in which case it is `udiv`.

## Interface

| function | meaning |
| --- | --- |
| `txt_setup()` | call once, after `init_nodes*()` and `ty_setup()` and `init_frame*()` |
| `txt_reset()` | start a new C function: clears the local table and `frame_reset()`s |
| `bind_local(name, ty)` → `int` | give a local a frame slot *and* record its C type; returns the offset |
| `ex_txt(id)` → `string` | the `id` text that evaluates expression node `id` |
| `addr_txt(id)` → `string` | the `id` text for the *address* of an lvalue node |
| `st_txt(id)` → `string` | one statement, semicolon included, no indentation |
| `ex_ty(id)` → `int` | the C type node of an expression — public because the CFG builder needs it for a condition's signedness |

`bind_local` replaces a bare `frame_add`. `emit/fr` records where a local
lives and how wide it is, which is all a frame layout needs; it does not
record **signedness**, and the emitter cannot do without it — a four-byte slot
is `peek32` either way, but `unsigned` divides with `udiv` and `int` with `/`.
So this module keeps a fourth list, `lvtys`, parallel to `fr`'s three, and
`bind_local` is what keeps them in step. `find_local`'s index is valid in all
four.

`st_txt` handles `expr`, `decl` and `return`. Everything else — `if`, `while`,
`do`, `for`, `switch`, `break`, `continue`, `goto`, `label`, `case` — is an
edge in the control flow graph rather than text and never reaches it.

## The translation, operator by operator

| C | `id` |
| --- | --- |
| `x` (int local) | `sx32(peek32(fp + 8))` |
| `x` (unsigned local) | `zx32(peek32(fp + 8))` |
| `c` (char local) | `sx8(peek8(fp + 8))` |
| `x = e` | `poke32(fp + 8, E)` — poke truncates, which is C's conversion on assignment |
| `&x` | `(fp + 8)` |
| `a + b` (int) | `sx32((A + B))` — the wrap is what makes `INT_MAX + 1` negative |
| `a + b` (unsigned) | `zx32((A + B))` |
| `a + b` (long) | `(A + B)` — already exactly 64 bits |
| `a / b` signed / unsigned | `(A / B)` / `zx32(udiv(A, B))` |
| `a % b` signed / unsigned | `(A % B)` / `zx32(umod(A, B))` |
| `a >> b` signed / unsigned | `(A >> B)` / `ushr(A, B)` |
| `a < b` signed / unsigned | `(A < B)` / `ult(A, B)` |
| `a > b` unsigned | `ult(B, A)` |
| `a <= b` unsigned | `(ult(B, A) == 0)` |
| `a >= b` unsigned | `(ult(A, B) == 0)` |
| `-e` | `sx32((0 - E))` — `id` has no unary minus |
| `!e` | `(E == 0)` — `id` has no `!` |
| `*p` | `peek32(P)` at the pointee's width |
| `p->f` | `peek32((P + 8))` |
| `s.f` | `peek32((fp + 8 + 8))` |
| `a[i]` | `peek32(((fp + 8) + (I * 4)))` |
| `p + n` | `(P + (N * 4))` — scaled by the pointee's size |
| `p - q` | `((P - Q) / 4)` |
| `(char)e` | `sx8(E)`; `(unsigned)e` → `zx32(E)`; to a pointer or any 8-byte type, nothing |
| `sizeof(T)` | the number — `sizeof` never survives into the output |
| `"abc"` | `mem_of_str("abc")` |
| `f(a, b)` | `c_f(A, B)` |
| `x += e`, `x++` | rewritten to `x = x + e`, so pointer scaling and wrapping happen once |

Every binary result is fully parenthesised, so `id`'s precedence never has to
be reasoned about — notably that its bitwise operators bind **tighter** than
its comparisons, which is the opposite of C, and `id` rejects the two mixed
without parentheses.

The table writes each form nested because that is how it reads. What is
emitted names every call first: `name_val` (`st/pend/name.id`) queues
`word tN = CALL;` and returns `tN`, and the block emitter prints the queue
before the statement that uses it (`flush_pend`). So `x + 1` for an int local is

```
word t0 = peek32(fp + 8);
word t1 = sx32(t0);
word t2 = sx32((t1 + 1));
```

and the expression text is `t2`. `pend`, `pendc` and `pendn` are set up in the
driver's init chain (`emit/gen/out/top/init/pend.id`). A store (`pokeN`) is a
statement and is never named.

## Two places where the obvious text is wrong

**A frame address is `fp + 24`, and that is not safe everywhere.** It is fine
inside `peek32(...)` and fine as the left operand of anything, but as the
right operand of a subtraction it reassociates: `q - p` on two decayed local
arrays would read `fp + 32 - fp + 8`, which is 32, not 24. So the parentheses
go on exactly where an address *escapes as a value* — `val_txt` for an array
or record lvalue, and `un2` for `&x` — and nowhere else. That keeps every
`peek32(fp + 24)` in the generated project spelled the way the reference
lowering in `tests/lowering/` spells it. This was a real bug, caught by
`&a[3] - &a[0]` disagreeing with `cc`.

**`id`'s `<<` is 32-bit when both operands are `int`-typed** and 64-bit as soon
as either is a `word`: `1 << 40` is 0, and `sx_bits(1, 64) << 40` is
1099511627776. Everything loaded from the store is already a `word`, so this
only bites when the left operand is a constant or is built out of constants —
`1UL << 40`, which is how the kernel spells every mask it has. `shl_arg`
therefore wraps a constant left operand in `sx_bits(v, 64)`, which is the
identity on the value and changes only its `id` type, so applying it where it
was not needed costs nothing.

## What it refuses to emit

`&&`, `||`, `?:` and the comma operator must not evaluate their right-hand
side unconditionally, and an `id` expression cannot express that. They become
extra basic blocks with a frame temporary, which only the CFG builder can
make. Reaching one emits `NEEDS_CFG_SHORTCIRCUIT`, an identifier no generated
project defines, so the build fails with a `file:line: error:` rather than the
program running with its side effects in the wrong order. The same goes for
`NOT_AN_LVALUE_*` and `UNKNOWN_MEMBER_*`: every gap in this module is a
compile error in the output, never a wrong number.

## Verified against `cc`

74 differential cases: the emitted text was wrapped in an `id` program,
compiled with `bin/idc`, run, and compared against the same C compiled with
`cc` and run. Covered: signed and unsigned division and modulo including
negative dividends, 32-bit wraparound of `int` and `unsigned int`, arithmetic
versus logical shift, unsigned comparison (`4000000000u <= 5u`), the
signed/unsigned comparison C converts (`-1 < 1u` is false), all six widths of
sign- and zero-extension, narrowing casts and cast chains, pointer arithmetic
on 1-, 4- and 8-byte element types, pointer differences and comparisons,
`&a[3] - &a[0]`, arrays of structs, `struct`/`union` member offsets against
`sizeof`/`offsetof`, pointer-to-pointer, and calls into `crt` (`memset`,
`memcpy`, `strlen`). Every one agrees.

## Known gaps

* **The call convention is the argument-passing one, not `EMITTER.md` §4's.**
  A call emits `c_NAME(args)`, which is exactly right for `crt` (C `memcpy`
  becomes `c_memcpy`, a function that already exists with C's semantics) and
  is the wrong shape for a translated C function, which needs its own frame
  allocated so that recursion and `&param` work. That thunk is a *function*,
  not an expression, and it needs the callee's frame size and driver name, so
  it belongs with `emit/cfg/`.
* **Indirect calls** emit `crt_call(...)`, a name nothing defines yet.
* **`x++` as a subexpression** emits the increment, not the value before it.
  That is right where the straight-line emitter meets one — as a whole
  expression statement — and wrong inside a larger expression, which needs a
  frame temporary.
* **A compound assignment emits its target's address twice**, once to read and
  once to write. Free for a local, duplicated subscript arithmetic for
  `a[i] += 1`, and wrong only when the address expression itself has a side
  effect (`*p++ += 1`).
* **A global variable** emits `g_NAME`. There is no data segment until the
  emitter lays one out.
* **Brace initialisers** (`int a[3] = {1,2,3}`) are not handled; the parser
  models them as a `cast` of a `call` with callee -1, and this module treats
  that as a call.
* **A string literal's escapes are passed through unchanged**, on the
  assumption that C and `id` spell the common ones the same way.
* **Floating point** is absent throughout, as it is everywhere else in `c2id`.

### One gap that is not this module's

`num_ty` reads a literal's suffix to decide its C type, and also wants to
widen a literal too big for `int`. It cannot: the node ABI stores a literal's
value in `na`, an `int[]`, and `id`'s `int` is 32 bits, so
`0x0123456789abcdefUL` arrives here already truncated to `-1985229329`. The
magnitude tests in `ty/k/k3/lit.id` are therefore unreachable today and
correct for the day `na` widens. The truncation happens in
`parse/code/ex/p/pf/lit/num/n3/digit_val.id`'s `digits_val`, well before any emission —
`81985529216486895UL` prints as `-1985229329` from `expr_str` alone.

## File map

```
ty/                         the C type of an expression
  std/reg/setup.id          txt_setup fill_std fill_std2
  std/types.id              ty_int ty_uint ty_long
  std/signedness.id         ty_ulong ty_charp ty_signed
  k/k1.id                   ex_ty ety2 ety3          -- one node kind each,
  k/mid.id                  ety4 ety5 ety6              a lazy chain: an eager
  k/k3/index.id             ety7 ety8 ety9              one would evaluate
  k/k3/end.id               ety10 ety11 ety12           every arm of every
  k/k3/lit.id               num_ty num_uty num_sty      node, exponentially
  r/un.id                   un_ty un_ty2 un_ty3
  r/bin.id                  bin_ty bin_ty2 bin_ty3
  r/c/conv/usual.id         usual_ty usual2 usual3
  r/c/pred.id               is_ptr elem_ty is_cmp_op
  r/c/m/misc.id             promote_ty has_ch local_ty
  r/c/m/loc.id              bind_local txt_reset decl_slot
  r/c/m/rec.id              rec_of mem_ty
ex/                         the text
  d/d1.id                   ex_txt ex2 ex3           -- the dispatch chain
  d/x/d2.id                 ex4 ex5 ex6
  d/x/d3.id                 ex7 ex8 ex9
  d/x/tail.id               ex10 lval_txt num_txt
  d/y/lit/atoms.id          str_txt cast_txt sizeof_txt
  d/y/call/expr.id          call_txt args_txt assign_txt
  d/y/asg/compound.id       comp_asg step_txt op_head
  a/addr.id                 addr_txt ad2 ad3         -- addresses
  a/m/ad4/addr.id           ad4 var_addr idx_addr
  a/m/mem/addr.id           mem_addr base_txt scale_txt
  a/m/val.id                val_txt
  a/w/wid/width.id          wid_bits load_wid cvt_name
  a/w/cv.id                 conv_txt load_txt store_txt
  a/w/un.id                 un_txt un2 un3
  o/b/bin/dispatch.id       bin_txt bin1 bin2        -- operators
  o/b/plain/arith.id        bin3 plain_txt shl_arg
  o/b/sgn/signed.id         sgn_txt uop_txt ufn_name
  o/p/ptr/arith.id          ptr_bin ptr_off ptr_off2
  o/p/diff/unary.id         ptr_diff un4 un_pre
  o/p/un.id                 pre_txt incdec_txt
  o/c/cmp/compare.id        cmp_txt cmp_sgn ucmp_txt
  o/c/cmp/unsigned.id       ult_txt ucmp_neg
st/st.id                    st_txt stt2 stt3
st/more.id                  decl_st ret_st
```

39 files, 111 functions.

## Names added to the registry

`NOTES.md` §3 governs; these are new, and all of them keep to a type that
file already fixes (`s`/`out` string, `ty`/`inner`/`off`/`sz`/`w`/`i`/`c`/`k`
int, `op` string, `words`/`names` string[]). Two exported globals are new:
`stdty` (`int[]`, the handful of C types the emitter names rather than reads)
and `lvtys` (`int[]`, one C type node per frame slot).

## Building and testing it

`c2id` still has no `main`, so this module is exercised by a scratch harness
outside the repo: copy `c2id/lex`, `c2id/parse`, `c2id/emit/fr` and this
directory beside a `main` that lexes and parses C statements from stdin and
prints `st_txt` of each, then feed the printed text to `bin/idc` and diff the
result against `cc`. Build the tree with the umbrella's `idc/bin/idc <dir>` —
it enforces all thirteen of the language's rules and reports
`file:line: error:`.
