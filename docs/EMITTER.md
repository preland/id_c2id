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
fp + …    parameter 1, …      (each padded to its alignment)
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
`t1`, ... on its own line (`emit/gen/txt/st/st/pend.id`), and the lines are
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
* **3 functions per file, 3 entries per directory**: guaranteed by §5.
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
