# Extensions `id` needs, and why each one is unavoidable

This is the contract between this repository and `../id_development`. Every
item below is justified by something in the C abstract machine that cannot be
expressed otherwise; anything that *could* be written in `id` is written in
`id` (in `crt/`) instead of being added to the language.

The guiding constraint: `id` is a small language with a point of view, and a
port should not turn it into C. Three additions, no more.

## 1. `word` — a 64-bit machine word

`id`'s `int` is C `int`: 32-bit and signed. The kernel needs 64-bit addresses,
`u64` counters, and defined wraparound. Rather than import C's entire integer
zoo (`u8`,`u16`,`u32`,`u64`,`s8`…), `id` gains **one** new base type:

```
word    // 64-bit, two's complement, wraps on overflow
```

Everything narrower is a `word` carrying a narrower value; `c2id` inserts the
truncation and sign-extension where C's rules demand it, using `crt` helpers
written in `id`. This keeps the type system at five types (`int`, `float`,
`string`, `void`, `word`) instead of twelve.

`word` maps to C `long long` and to a 64-bit integer in the LLVM and WASM
backends.

**Signedness.** The operators that do not care about signedness
(`+ - * & | ^ ~ <<`) are plain operators. The four operations where signed and
unsigned genuinely differ get *unsigned* builtins, with the plain operator
keeping its signed meaning:

| signed | unsigned |
| --- | --- |
| `a / b` | `udiv(a, b)` |
| `a % b` | `umod(a, b)` |
| `a < b` | `ult(a, b)` |
| `a >> b` | `ushr(a, b)` |

Four builtins is a much smaller tax than a parallel set of unsigned types, and
it makes the signedness of every kernel operation *visible at the call site*,
which is a genuine readability win over C — where `>>` silently means two
different things depending on a declaration three files away.

## 2. Bitwise operators

```
a & b    a | b    a ^ b    ~a    a << b    a >> b
```

Non-negotiable. Kernel code is bit manipulation: flags, masks, page-table
entries, atomic bit operations, hash functions, checksums. There is no way to
express `PAGE_MASK` without them, and emulating them with division would be
both unreadable and unusably slow.

Precedence follows C, slotted into `id`'s existing table below the comparisons
so that `flags & MASK != 0` parses the way a systems programmer expects
(`(flags & MASK) != 0`), rather than C's historical mistake. This is a small,
deliberate divergence from C and `c2id` fully parenthesises its output anyway,
so it never bites the port.

### Shift width follows the operand type

`a << b` is a 32-bit shift when both operands are `int`, and 64-bit as soon as
either is a `word`. So `1 << 40` is 0 and `w << 40` (with `w` a `word`) is
1099511627776 — exactly what C does, and exactly the trap C has.

This is worth stating because it is how systems code spells every mask it
owns. `1UL << 40` in C is a `word` shift; writing it as `1 << 40` in `id`
gives zero, silently. When the left operand is a constant, widen it — any
`word`-typed expression will do.

## 3. A flat, bounds-checked memory store

The one genuinely new capability. Six builtins:

```
alloc(word nbytes) -> word     // zeroed; returns a base address (never 0)
peek8(word addr)   -> word     // zero-extended load
peek16 / peek32 / peek64
poke8(word addr, word value)   // truncating store
poke16 / poke32 / poke64
```

Addresses are opaque integers. Every access is **bounds-checked against the
live allocation set**; an out-of-range address aborts with a diagnostic in
exactly the way `id`'s existing list indexing does. This is what makes structs,
unions, pointer arithmetic, arrays, and `&x` expressible, and it is why the
resulting kernel port is memory-safe: the class of bug that produces CVEs in
the real kernel produces a clean abort here.

Two bridges to `id`'s existing world, since a port must still be able to print:

```
str_of_mem(word addr, word len) -> string
mem_of_str(string s)            -> word    // copies into the store, NUL-terminated
```

### Why not lists?

`id` already has `T[]`. It is the wrong primitive here: cells are boxed to 8
bytes and typed, so a `struct` with a `u8` next to a `u32` cannot be laid out,
`&x` has no meaning, and casting a `struct sock *` to a `struct sock_common *`
— which the kernel does constantly — is inexpressible. Flat bytes are the
thing C actually assumes.

### Why not `free`?

The store is an arena. `id`'s existing C backend already allocates and never
frees for string concatenation, and a kernel port is a whole-program
translation where lifetimes come from the translated code, not from `id`.
`crt` implements `kmalloc`/`kfree` as a free-list **inside** a single `alloc`ed
region, written in `id` — so the kernel's allocator is ported rather than
assumed.

## 4. Deliberately *not* added

Each of these was considered and rejected, because `c2id` can lower it:

* **structs / records** — offsets computed at translation time
* **pointers** — `word` addresses
* **`for`, `switch`, `break`, `continue`, `goto`** — edges in the CFG
  (`docs/DESIGN.md` §2)
* **function pointers** — integer ids plus a generated dispatch tree
* **casts** — `crt` truncation/extension helpers
* **variadic functions** — `printk` takes a `word[]` of arguments
* **`const`, `volatile`, `static`, `inline`** — no runtime meaning here
* **increased action / nesting / file limits** — the limits are the language's
  entire point. Machine-generated code has no business asking for an exemption
  that human-written code does not get; `c2id` splits its output to fit, and
  the fact that it *can* is the strongest evidence that the limits are
  workable.

## 5. Compatibility

Every addition is additive in behaviour: no existing `id` program changes what
it *computes*. `int` arithmetic in particular is emitted exactly as before —
the checked division and shift helpers apply only to `word`, which is new, so
that nothing existing pays for them and byte-for-byte codegen parity with the
self-hosted compiler is preserved.

There is one real incompatibility: **`word` is now a keyword**, so a program
using `word` as an identifier no longer compiles. One did —
`demos/idc_in_id/scan/scan.id` held its accumulated identifier text in a
variable called `word` — and it was renamed to `ident`. That is the entire
observable break, and it is the unavoidable cost of adding any keyword at all.

Hex literals (`0xdeadbeef`) were added at the same time. They are a spelling,
not a type: `0xff` lexes to the integer token `255` and nothing downstream
knows the difference. Without them, mask constants would have to be written in
decimal, which makes bit-manipulation code unreadable for no gain.

## 6. Implementation ledger

Legend: ✅ implemented and tested · ⛔ rejected with a clear diagnostic
(deliberate — see below) · ⏳ not yet.

| Item | `idc.py` (C) | LLVM | WASM | self-hosted |
| --- | --- | --- | --- | --- |
| hex literals | ✅ | ✅ | ✅ | ✅ |
| bitwise `& \| ^ ~ << >>` on `int` | ✅ | ✅ | ✅ | ✅ |
| `word` (64-bit machine word) | ✅ | ⛔ | ⛔ | ✅ |
| `alloc` / `peek*` / `poke*` | ✅ | ⛔ | ⛔ | ✅ |
| `udiv` / `umod` / `ult` / `ushr` | ✅ | ⛔ | ⛔ | ✅ |
| `str_of_mem` / `mem_of_str` | ✅ | ⛔ | ⛔ | ✅ |

**Why LLVM and WASM reject rather than implement.** A C pointer, an LLVM
`inttoptr`+`load`, and a WASM `i32.load8_u` into linear memory are three
genuinely different address spaces, and WASM's runtime is a separate
hand-written 337-line WAT implementation that would need a parallel `i64`
instruction set. Guessing at those semantics would be worse than saying
plainly that the flat store is C-only, which is what those backends now do —
with a diagnostic naming the feature, not a compiler crash. The bitwise
operators, which are cheap and unambiguous, *are* on all three, and the test
suite checks all three produce identical output.

The self-hosted column is now complete: `bin/idc` covers the whole language,
enforces all thirteen of its rules itself, and has no fallback to `idc.py`.

**Historical note — why the self-hosted lexer/parser lagged.** `bin/idc` already falls back to
`idc.py` transparently when the self-hosted stages cannot handle an input, so
programs using the new syntax build correctly today — just via the reference
compiler, with a note on stderr. Closing this gap means adding the precedence
ladder to a parser written in `id` under the 3-action rule; it is tracked in
`docs/STATUS.md` and does not block the port.
