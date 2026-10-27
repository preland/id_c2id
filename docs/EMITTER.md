# The emitter: from AST to `id`

This is the specification for `c2id/emit/`. It is written out in full because
the emitter is where the design in `docs/DESIGN.md` becomes real code, and
because `tests/lowering/` already shows exactly what the output must look
like — that worked example is normative, this document explains it.

## 0. Shape of the output

For each C function, the emitter produces:

* one `id` function per **basic block**, `blkN(word fp)`, returning the number
  of the next block (`0 - 1` means "the function is done");
* a **dispatch tree** mapping a block number to its block function;
* a **driver** that loops until a block returns the end marker.

Everything else — `goto`, `break`, `continue`, `switch`, every loop form — is
an edge in the graph and needs no separate handling.

## 1. Frames

A C function's parameters and locals live in the flat store, at a frame
allocated on entry. Three reasons, in order of force:

1. `id` variables are function-private, so a local written in block 3 and read
   in block 9 has nowhere else to live.
2. It makes `&local` mean something, for free.
3. It makes recursion work: each call allocates its own frame.

Layout, assigned at translation time and emitted as literal constants:

```
fp + 0    return value        (8 bytes, always)
fp + 8    parameter 0
fp + …    parameter 1, …      (8 bytes each; a struct or union by value takes its size, to 8)
fp + …    locals in declaration order
```

`emit/fr/` walks a function's AST once, assigns every parameter and local an
offset from `type_size`/`type_align`, and records the total frame size. A
local declared inside a nested block still gets a frame slot — C's block
scoping is a naming rule, and the naming has already been resolved by then.

**Access width follows the C type**: a `char` local is `peek8`/`poke8`, an
`int` is `peek32`, a pointer or `long` is `peek64`. A value loaded from a
signed type narrower than 64 bits must be sign-extended with the `crt`
helpers (`sx8`, `sx16`, `sx32`); an unsigned one zero-extended (`zx8`, …).
Getting this wrong is the single most likely source of silent wrong answers,
so the differential tests in `tests/c/010_arith.c` and `020_bitwise.c` exist
specifically to catch it.

## 2. Building the CFG

`emit/cfg/` turns a function body into a list of blocks. A block is a list of
straight-line statements plus a terminator.

Walk the statement tree, maintaining:

* the block currently being filled;
* a **break target** and a **continue target** stack (as parallel `int[]`
  lists, since `id` has no records) — pushed on entering a loop or `switch`,
  popped on leaving;
* a **label table**, name → block number, plus a fix-up list of `goto`s whose
  target had not been seen yet.

Per construct:

| C | blocks |
| --- | --- |
| straight-line statements | appended to the current block |
| `if (c) A else B` | terminator = conditional edge to A-entry / B-entry; both fall to a join block |
| `while (c) B` | cond block; body; back-edge to cond. break → exit, continue → cond |
| `do B while (c)` | body; cond block; back-edge to body |
| `for (i; c; s) B` | init in the current block; cond block; body; step block; back-edge to cond. continue → **step**, not cond |
| `switch (v) B` | a dispatch chain of equality tests against each `case` value, defaulting to the `default` block or the exit. break → exit |
| `break` / `continue` | unconditional edge to the top of the relevant stack |
| `goto L` | unconditional edge to L's block, resolved by the fix-up pass |
| `label L:` | starts a new block, recorded in the label table |
| `return e` | store to `fp + 0`, terminator = the end marker |

Note the `for`/`continue` subtlety: `continue` in a `for` must run the step
clause. That is one of the few places where a mechanical translation can be
silently wrong, and `tests/c/030_control.c` covers it.

## 3. Emitting a block

Statements become straight-line `id`. Because a block never nests, the only
constraint is the 3-action limit, and a block with more than 3 actions is
split into a chain of ordinary functions called in sequence:

```
blk7(word fp) {
  poke32(fp + 16, ...);
  poke32(fp + 24, ...);
  blk7b(fp);
} return word 8;

blk7b(word fp) {
  ...
} return void;
```

No control flow is involved in the split, so it is trivially correct.

**Expressions** are emitted as a chain of named temporaries, one call per
statement, with `crt` helpers for anything C does that `id` does not.

The table below writes each form nested, which is how it reads, but **nested is
not what may be emitted**: `docs/SPEC.md` says a call may not be an argument to
a call at any depth, and a return clause is a name or a literal. So a local `x`
is not `sx32(peek32(fp + OFF))` but

```
word t0 = peek32(fp + OFF);
word t1 = sx32(t0);
```

Every call the expression builders write is bound to a `word` temporary `t0`,
`t1`, ... on its own line (`emit/gen/txt/st/pend/name.id`), and the lines are
printed just before the statement or terminator that uses them. Temporaries
restart at `t0` in each block function. A C expression statement takes its last
temporary back, so `printf(...)` is emitted as `c_printf(t0, t5);`, not as a
temporary nobody reads. A store is not named -- it is a statement -- so a C
assignment or `++` used *as a value* still emits a `pokeN` inside an expression,
which `bin/idc` rejects; giving it C's value (the stored value, or the old one
for postfix) is not done yet.

This was written the other way for a long time, and the emitter was built to
match, because `c2id` was compiled by `idc.py` and `idc.py` does not enforce
either rule -- see the umbrella's `docs/GAPS.md` B6/B7. The specification was
wrong, not the compiler.

The consequence is not only cosmetic. **A temporary is an action, and a block
gets three**, so the block splitting in section 3 has to be driven by the number
of *emitted* statements rather than the number of C statements -- one C
assignment can be five. Splitting on the C count would emit blocks that do not
compile, and it would do so only for expressions past a certain depth, which is
the worst way to find out.

| C | `id` |
| --- | --- |
| `x` (local) | `peek32(fp + OFF)`, wrapped in `sx32(...)` if signed and narrower than 64 |
| `x = e` | `poke32(fp + OFF, e)` |
| `*p` | `peek32(p)` at the pointee's width |
| `p->f` | `peek32(p + OFF_f)` |
| `a[i]` | `peek32(a + i * ELEMSIZE)` |
| `&x` | `fp + OFF` |
| `p + n` (pointer) | `p + n * sizeof(*p)` |
| `a / b` unsigned | `udiv(a, b)` |
| `a >> b` unsigned | `ushr(a, b)` |
| `(char)e` | `sx8(e)` |
| `f(a, b)` | `c_f(frame_for_call)` — see below |
| `e1 && e2` | must **short-circuit**, so it becomes two blocks, not one expression |

`&&`, `||` and `?:` short-circuit, which an expression cannot express when the
right-hand side has side effects. They are lowered to extra blocks with a
temporary in the frame. This is the one place where expression emission has
to reach back into the CFG builder.

## 4. Calls

A call allocates the callee's frame, stores the arguments, calls the callee's
driver, and reads `fp + 0`:

```
crt_call_f(word fp) {
  word nfp = alloc(FRAMESIZE_f);
  poke64(nfp + 8, arg0);
  poke64(nfp + 16, arg1);
} return word c_f(nfp);
```

**Indirect calls.** An address-taken C function is assigned a small integer
id. `ops->read(a, b, c)` becomes `crt_call3(id, a, b, c)`, where `crt_callN`
is a dispatch tree over every address-taken function of that arity, generated
by the emitter. `struct file_operations` and its relatives then work
unchanged.

## 4b. Struct values

A struct or union is an address everywhere in an expression (the value of a
record is its own address, `val_txt`), so passing and returning one is passing
and returning an address plus a copy at the right place.

**By value in.** The caller passes the address of the object. The callee's
parameter slot is the object itself: `par_slot` makes it the type's size rounded
up to 8 bytes instead of one 8-byte word, the offsets of the parameters after it
move with it (`par_off`), and the wrapper `c_NAME` copies the caller's bytes into
the slot with `c_memcpy` before the function runs (`wrap_copy`). The callee owns
that copy, so a write to the parameter never reaches the caller's object. The
copy is made when the call is made, after every argument has been evaluated.

**By value out.** A function whose return type is a struct or union has a hidden
first parameter, a pointer to the caller's result object (`fn_node`): its type
lists it before the named parameters, so the wrapper, the `crt_callN` dispatch and
the frame all count it as parameter 0, at `fp + 8`. The frame pass gives each call
of such a function a frame temporary of its own and passes its address as that
argument (`sret_arg`), so `f(g(), g())` and a recursion never share an object. An
end block stores `c_memcpy(peek64(fp + 8), value, size)`, which answers its
destination, at `fp + 0` (`ret_copy`), so the call's value is the temporary's
address, and `f().x` and the call as an initialiser, an argument or an assignment
source all read it like any other record. The wrapper also stores the hidden
argument at `fp + 0` as the frame is made, and a block that ends the function
without a value (a call that never returns) answers it, so a callee that exits
leaves the caller a pointer to its own temporary.

**Still refused.** A struct or union passed through the `...` of a variadic call
(`printf("%d", s)`): the extra arguments are a list of words.

## 4c. Variadic definitions

A function defined with `...` takes its extra arguments the way the runtime's
`printf` does: the call site passes the fixed arguments, then the rest as one
`word[]` (`va_any`, `back/link/ext/call/`), and the wrapper `c_NAME(a0, ..., word[] va)`
(`back/link/fn/wrap/va.id`) copies them into one 8-byte slot each on the stack
above the callee's frame (`rt_va_pack`, `rt/ext/builtin/va.id`), behind a word
holding the address past the last. The function has one more parameter than its
type lists, `char *__va`, in the slot after the named ones (`bind_va`), and
`include/stdarg.h` is plain C over it: `va_list` is a one-element array of
`{cur, end}`, `va_start` sets both from `__va`, `va_arg(ap, T)` is
`*(T *)((ap->cur += S) - S)` with `S` the size of `T` rounded up to 8 (a
`long double` takes two slots). A `va_list` passed to another function therefore
shares its position, as C's does, and `vsnprintf`, `vsprintf`, `vprintf` and
`vfprintf` (`rt/ext/stdio/vprint.id`) hand the words left to the formatter. The
wrapper gives the slots back after the function returns (`rt_va_drop`).
`float` arguments are promoted at the call (a double reads back), and the
address of such a function cannot be taken (`mid/check/vaval.id`): a call through
a pointer passes fixed slots only.

## 4d. Bit-fields

The layout was always computed (`bf_pos`: a field goes at the cursor if it still
fits in an aligned unit of its declared type, else it starts the next one;
`: 0` forces that). An access goes through a marker in place of an address,
`BF|<unit address>|<unit bytes>|<bit>|<width>|<kind>` (`back/emit/gen/txt/ex/a/bf/`),
which `load_txt` and `store_raw` read: a load is the unit, shifted down, cut to the
width and, for a signed type, sign-extended from the field's top bit; a store reads
the unit, clears the field, puts in the value cut to the width (0 or 1 for a
`_Bool`) and writes the unit back. A field narrower than an int is an int in an
expression (`bfx_prom`, in `mem_ty`), as C says. Brace initialisers take the
same path and skip unnamed fields (`skip_unnamed`). Refused: a field wider than its
type. A field of enum type is unsigned unless an enumerator is negative, as the type of the enum is
(`parse/ty/dcl/e/more/enumty.id`).

## 4e. setjmp and longjmp

A call is an id call and id cannot cut its stack short, so `longjmp` does what
`exit` does: it makes every runner stop at its next block (`rtxc` holds -1, and
`longjmp` is on `nrnames`/`mxnames`, so a statement calling anything that may
reach it is followed by a block boundary), and the runner of the function that
called `setjmp` takes the jump. `setjmp(env)` is not a call: the lowering
(`mid/lower/mk/jmp.id`) makes a frame temporary `t`, stores the frame, a new
block's number and `&t` in `env` (`__c2id_setjmp`, `rt/libc/jump.id`), ends the
block with a jump to the new one and answers `t`; `longjmp` stores its value in
`t` (1 for 0) and a function that has such a block gets a runner with a second
loop that asks `rt_resume` whether a jump was to its frame
(`back/link/fn/tree/jump.id`). `sigsetjmp`/`siglongjmp`, `_setjmp`/`_longjmp` are the
same. Locals keep their latest values (they live in the frame), which C allows.

## 4f. Files written

A file opened for anything but plain reading is a longer FILE of kind 2: eight words, [data address, size, position, kind, capacity, a NUL-terminated copy of the path, flags, the next writable FILE open] (`rt/ext/stdio/file/wopen.id`). Its contents are held whole in the heap, like a file opened for reading: `r+` and `a` load what is on disk, `w` and `w+` empty the file at once. Writes (`rt_wput`) go to the position, or to the end for `a`, growing the buffer, and mark the FILE unsaved; `fflush`, `fclose` and the end of the program (`rt_status`, over the chain `rtwf`) save it with `fs_save`. `tmpfile` is a `w+` file with a flag that deletes it when it is closed. The cost is that a file is saved whole each time it is flushed, and that another reader sees nothing until then.

## 4g. Atomic builtins and the address of runtime functions

The frame pass (`mid/pass/frame/atom/`) rewrites a call of a GCC `__atomic_*` or `__sync_*` builtin into a call of `c___c2id_load`, `_store`, `_rmw`, `_cas` and the others in `rt/ext/builtin/atom*.id`, adding the size of the object the first argument points at (and an operation code) and casting the answer to that object's type; the order arguments are dropped, since the program has one thread. A runtime function used as a value is rewritten by the check pass to its C wrapper `__c2id_w_NAME` from the headers (`mid/check/wrap.id`), because a pointer is a place in the program's own function table.

## 4h. Long double

`long double` is the x86-64 80-bit extended format: 16 bytes at alignment 16 (so
`sizeof` and a struct's layout are gcc's), whose first ten bytes are the 64-bit
significand with its integer bit explicit and then the sign and the 15-bit biased
exponent; the other six are zero. No `word` holds one, so it is handled as a
struct is: `ty_agg` is 1 for it, its value in an expression is the address of
its 16 bytes (`val_txt`), a store copies them (`store_plain`), a parameter takes
a 16-byte slot copied from the caller's object (`par_slot`, `wrap_copy`), and a
function returning one has the hidden result pointer (`fn_node`, `sret_arg`).

Arithmetic, conversion and comparison are `crt/ldbl` (`cld_add`, `cld_sub`,
`cld_mul`, `cld_div`, `cld_neg`, `cld_cmp`, `cld_nz`, `cld_of_int`,
`cld_of_uint`, `cld_of_dbl`, `cld_to_int`, `cld_to_uint`, `cld_to_dbl`,
`cld_to_flt`, `cld_lit`), written to the address of a 16-byte frame temporary
the emitter makes for each use (`ld_tmp`: the slot is grown into the frame while
the blocks are printed, which is allowed because `emit_tail` runs after them; a
global initializer that needs one gets a frame of its own, `emit_gframe`), and
answered. `ty_fkind` is 3. The usual conversions make the pair a `long double`
when either operand is one (`fl_join`), so `ld + 1`, `ld * 2.5` and `ld < n`
convert the other side first. A literal with the suffix `l` or `L` is `cld_lit`
of its text (`ld_lit_txt`): the decimal is rounded once, exactly, to nearest even
at run time. Each of `+ - * /` is rounded once to nearest even at 64 bits, as the
x87 does, with its NaN, infinity, zero and denormal rules; `(double)` and
`(float)` of one round once from the 64-bit significand (not through a double),
a conversion to an integer truncates toward zero and gives the x87's "integer
indefinite" (the word `0x8000000000000000`) when the value does not fit.

Through `...` a `long double` is two words (`va_item`): printf's `%Lf %Le %Lg %LF
%LE %LG` read them (`conv_fl`, with the exact digits as for a double), and a
user function reads it with `va_arg(ap, long double)`. `strtold` is
`rt/ext/num/ldbl.id`; `float.h` has the `LDBL_` constants.

Not provided, and visible when used: the `l` libm functions (`sqrtl`, `fabsl`, ...
are not declared, so a call is the usual "neither defined nor provided" refusal
with `file:line`), `%La`, `sscanf` with `%Lf`, and printing the x87's invalid
encodings (unnormals, pseudo-NaNs and pseudo-infinities); arithmetic, comparison
and conversion do treat them as the hardware does (the indefinite NaN, or
unordered). The 16-byte alignment is in the layout, not in the addresses: the
store hands out 8-aligned addresses.

## 4i. 128-bit integers

`__int128` and `unsigned __int128` are 16 bytes at alignment 16 and are handled
as a long double is: `ty_agg` is 1, the value in an expression is the address of
its bytes (low word first), a store copies them, and a parameter or result is a
16-byte slot (`wrap_ty_kind`). The two type nodes are made on first use
(`wd_types`), and `wd_kind` answers 1 or 2 for them, 0 for anything else.

Every operation is `crt/i128`, given the address of a 16-byte frame temporary
for the result and the addresses of its operands, and answering the temporary:
`ci128_add sub mul neg not and or xor shl shr sar divu modu divs mods cmps cmpu
nz lo of_s of_u`. Arithmetic wraps modulo 2^128; a shift count outside 0 to 127
is as undefined as in C. The low word is `peek64` of the value's address, where a
conversion to any narrower integer starts, and `ci128_of_s` or `ci128_of_u` widen a word by sign
or zero extension. The result of a comparison, `&&`, `||` and `!` is an `int`.
Division by zero is undefined in C; here the quotient is 0 and the remainder
the dividend. The routines were compared with gcc over four million random
operand pairs in all.

Not tested, and possibly wrong: `__int128` bit-fields and `__int128` through
`...`. A literal is at most a word.

## 5. Laying out the project

`emit/gen/out/` already implements this and it is tested. Function number *N*
goes in file *N*/3, which goes in leaf directory *N*/9, whose path is that
number written in base 3 — one directory level per digit, at a fixed depth so
no directory ever mixes files with subdirectories. See
`c2id/emit/gen/out/path.id`.

The emitter writes the whole project to stdout as a stream of
`==== FILE <path>` sections; `tools/c2id.sh` splits it.

## 6. Staying legal under `id`'s own rules

The generated project has to obey every rule that hand-written `id` does —
that constraint is the point, and `tests/lowering/` proves it is met.

* **3 actions**: guaranteed by the block-splitting in §3.
* **Nesting 2**: a block function is straight-line; the driver loop is depth 1;
  a dispatch function is one `if`/`else`.
* **3 functions per file** (and the generator's three-entries-per-directory
  layout, a since-dropped rule): guaranteed by §5.
* **One name, one type**: generated code uses `fp`, `nxt` and `pc` and the
  temporaries `t0`, `t1`, ... — every one of them a `word`. Nothing else.
* **No constant function**: a block with no statements that only jumps, or
  whose condition is a literal, would be a function with no effect and a
  constant result. The emitter threads edges past such blocks and does not
  print them, and an end block with no return value stores 0 at `fp`. A cycle
  made only of empty jumps (`for (;;);`) is the one such block still emitted.
* **Function uniqueness** — the one that needs care. Two C functions with
  identical bodies would generate identical `id`. Every block function ends
  with its successor's number as a literal, and literals distinguish
  functions, so blocks in different positions already differ. Two genuinely
  identical *whole functions* still collide; the emitter must detect that and
  emit one, with the second name calling it. `bin/idc` enforces this rule,
  and `bin/idc` is the only compiler anything here is built with.

## 7. Order of work

1. `emit/fr/` — frame layout. Testable alone: print the layout for a function
   and check offsets against `cc`'s.
2. `emit/gen/txt/` — expression emission for the straight-line subset, with
   the CFG stubbed to one block. This gets `tests/c/010_arith.c` passing.
3. `emit/cfg/` — real blocks. `030_control.c`.
4. Calls and frames for recursion. `070_recursion.c`.
5. Pointers, structs, function pointers. `040`–`060`.

Each step is a row in `docs/STATUS.md` and a passing case in `tests/run.sh`,
or it did not happen.
