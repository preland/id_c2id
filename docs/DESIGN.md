# Porting Linux to `id` — architecture

## 0. The problem, stated honestly

`id` (see `../id_development`) is a deliberately tiny language:

* at most **3 actions per block** — and every block counts: the function body,
  each `if`, each `else`, each `while` body;
* at most **2 levels of block nesting**;
* at most **3 functions per file**, at most **3 entries per directory**;
* **a name has exactly one type program-wide**;
* variables are function-private unless `export`ed and read via `(import x)`;
* the return type is declared *after* the closing brace: `} return int 0;`
* types are `int` (C `int`), `float` (C `double`), `string` (C `char*`),
  `void`, and `T[]` (a boxed, growable, reference-semantics list).

The Linux kernel is C. It needs structs, unions, pointers, pointer arithmetic,
sized and unsigned integers with wraparound, bitwise operators, arrays,
function pointers, `for`/`switch`/`break`/`continue`/`goto`, casts, variadic
functions, and macros. `id` has **none** of these.

So a port is not a translation exercise. It is a compiler-construction
exercise, in three parts:

1. **extend `id`** by the smallest coherent set of primitives that makes the C
   abstract machine expressible at all (`docs/ID_EXTENSIONS.md`);
2. **write `c2id`**, a C→`id` compiler, *in `id`*, which lowers C onto those
   primitives while respecting every one of `id`'s structural limits;
3. **run it over the kernel**, outward from the most freestanding code.

The rest of this document is the design that makes (2) tractable.

## 1. The machine model: flat memory + one machine word

The single decision that makes everything else fall out is: **do not try to
map C's type system onto `id`'s type system.** Instead, model the C abstract
machine directly — a flat, byte-addressed memory plus 64-bit machine words —
and let *all* of C's structural richness become address arithmetic.

Under that model:

| C construct | becomes |
| --- | --- |
| `int x` (local) | 4 bytes at a fixed offset in the function's frame |
| `&x` | the frame address plus that offset — an ordinary integer |
| `p->field` | `peek32(p + OFF_field)` |
| `a[i]` | `peek32(a + i * 4)` |
| `struct` | nothing at all — just offsets, computed at translation time |
| `union` | nothing at all — overlapping offsets |
| bitfield | a shift and a mask |
| `char *s` | an address |
| function pointer | a small integer, dispatched by a generated function |

There is no need for records, pointers, or generics *in the language*. There
is a need for exactly one wide integer type and a bounds-checked flat store.
That is a far smaller ask of `id`, and it keeps `id` recognisably itself.

A pleasant consequence: because every load and store goes through a
bounds-checked primitive, **the resulting kernel is memory-safe by
construction** — an out-of-bounds access traps instead of corrupting.

### 1.1 What `id` gains

Deliberately minimal (details and rationale in `docs/ID_EXTENSIONS.md`):

* one new base type, **`word`** — a 64-bit two's-complement machine word;
* the **bitwise operators** `& | ^ ~ << >>`, which C cannot live without;
* a **flat memory store** with `alloc`, `peek8/16/32/64`, `poke8/16/32/64`.

Signedness is handled without doubling the type system: the plain operators
are the signedness-agnostic or signed ones (`+ - * & | ^ ~ <<`, and signed
`/ % < <= > >= >>`), and the four operations where unsigned differs get
builtins — `udiv`, `umod`, `ult`, `ushr`. Everything else C needs
(sign-extension, truncation to 8/16/32 bits, unsigned widening, saturating
helpers) is written **in `id`** in the `crt/` runtime, not baked into the
compiler.

## 2. Lowering C to `id`: blocks as functions

The second decision is how to survive "3 actions per block, 2 levels of
nesting" when a single kernel function can be four hundred statements deep
inside five levels of loops and `goto`s.

The answer is to stop treating the C function as the unit of translation and
use the **basic block** instead. `c2id` compiles each C function to a control
flow graph and emits:

* **one `id` function per basic block**, taking a frame pointer and returning
  the *number of the next block*;
* **one dispatch tree** mapping a block number to its function;
* **one driver loop** that runs the dispatch until the function returns.

```
f(word fp) {
  word pc = 0;
  while (pc != BLOCK_DONE) {
    pc = f_disp(fp, pc);
  }
} return word peek64(fp + FRAME_RET);
```

This is the whole trick, and it pays for itself several times over:

* **`goto` is free.** A jump is `return word 12;`. The kernel's pervasive
  `goto err_unlock;` idiom needs no structural analysis at all.
* **`break`, `continue`, `switch`, `for`, `do/while`** are all just edges in
  the graph — no special cases in the emitter.
* **Nesting is bounded by construction.** A basic block is straight-line code;
  it never nests. The driver loop is depth 1, the dispatch tree depth 2.
* **The 3-action limit becomes a purely local concern.** A block with 30
  statements is split into a chain `blk7`, `blk7b`, `blk7c`, … of at most 3
  actions each, called in sequence. No control flow is involved, so the split
  is trivially correct.
* **The stack does not grow.** Blocks return to the driver loop rather than
  tail-calling each other, so a million-iteration loop uses constant stack —
  which matters, because `id` has no tail-call guarantee.

### 2.1 Frames: why locals live in memory

Every block function takes exactly one parameter, `word fp`, and every C local
lives at a fixed offset inside that frame. Nothing else would work: `id`
variables are function-private, so a local that is written in block 3 and read
in block 9 has nowhere else to live. Putting frames in the flat store also
gives `&local` for free, and makes recursion natural (each call allocates a
new frame).

```
   fp + 0   saved return value
   fp + 8   parameter 0
   fp + 16  parameter 1
   fp + 24  local `i`
   fp + 28  local `err`
   ...
```

Offsets are assigned by `c2id` at translation time and emitted as literal
constants, so there is no runtime frame-layout cost.

### 2.2 Function pointers

An address-taken C function is assigned a small integer id. A call through a
pointer becomes `crt_call3(id, a0, a1, a2)`, where `crt_callN` is a dispatch
tree generated by `c2id` over every function whose address is taken with that
arity. `struct file_operations` and friends therefore work unchanged.

### 2.3 Preprocessing

`c2id` consumes **preprocessed** C. The kernel's macro layer is inseparable
from its build system (`Kconfig`, per-arch headers, `-include` files), so the
driver in `tools/` runs the real preprocessor to produce a single translation
unit and hands that to `c2id`. This is the same pragmatic split `id` itself
makes with `bin/idc`: the language has no filesystem or subprocess builtins,
so a thin non-`id` driver layer is legitimate and expected.

Inline assembly cannot be translated. Each `asm` block is matched against a
table of hand-written `crt` intrinsics (barriers, atomics, `cpuid`, port I/O);
anything unmatched is reported as an explicit, countable gap rather than
silently mistranslated.

## 3. Layout under the rule of 3

`id`'s "at most 3 entries per directory" turns a project into a ternary tree.
A program with N functions needs a tree of depth about log₃(N/3). `c2id`
generates this automatically: it fills a directory with 3 `.id` files (3
functions each = 9 functions), then starts a subdirectory, and so on. Names
carry their path (`b/b1/b1_2.id`) so a generated function's location is
derivable from its number, which keeps the emitter stateless.

## 4. Verification: differential testing, not faith

Nothing here is trustworthy without evidence, so the harness in `tests/` does
the same thing for every case:

1. compile the C with `cc`, run it, record stdout and exit status;
2. translate the C with `c2id`, build the `id` with `idc`, run it;
3. require the two to agree byte for byte.

The corpus starts at "does arithmetic wrap correctly" and grows toward whole
kernel library files. A port claim is only ever as good as the case that
demonstrates it, and `docs/STATUS.md` tracks exactly which files pass, which
fail, and why.

## 5. Order of attack

Kernel code is not uniformly hard. The order below is by decreasing
self-containment — each stage needs strictly less of the environment than the
next.

1. `lib/` leaf algorithms — `sort.c`, `list_sort.c`, `bsearch.c`, `string.c`,
   `bitmap.c`, checksums, hashes. Pure computation over memory; no kernel
   services at all. This is where the port is proved.
2. `lib/` data structures — rbtree, radix tree, xarray, idr.
3. Kernel core, algorithmic parts — schedulers' data structures, time
   conversion, kfifo.
4. Anything touching hardware, per-CPU state, or inline asm — needs `crt`
   intrinsics and is where the honest gaps will be.

`docs/STATUS.md` is the ledger.
