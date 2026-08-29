# `crt` — C semantics, written in `id`

`crt` is the runtime layer that gives translated C its behaviour. It is a
plain `id` project tree (3 entries per directory, 3 functions per file,
3 actions per block, nesting depth 2 — no exemptions), and it is meant to be
concatenated with `c2id`'s generated code at build time so that everything
compiles as one `id` program.

Nothing here is a language extension. Everything is written in `id` on top of
the primitives `docs/ID_EXTENSIONS.md` describes: `word`, the bitwise
operators, the flat bounds-checked store (`alloc`, `peek*`, `poke*`,
`str_of_mem`, `mem_of_str`) and the four unsigned builtins (`udiv`, `umod`,
`ult`, `ushr`). That is the point: if a piece of C's abstract machine can be
expressed in `id`, it is expressed in `id` and not baked into the compiler.

Every value below is a `word` (64-bit, two's complement) or a store address,
unless it says otherwise.

## Build and test

`crt` has no `main`, so it does not build on its own. Drop it beside a
directory containing a `main` and build the pair:

```sh
python3 ../idc/idc.py <project-dir> -o /tmp/out
```

Use `idc.py`, never `bin/idc`: only the reference compiler enforces the
action limit, the nesting limit, the one-name-one-type rule and the
duplicate-logic rule, which is the whole claim being made about this tree.

## File map

```
crt/
├── int/                    integer width semantics
│   ├── sx.id               sx_bits, sx8, sx16
│   ├── wide.id             sx32, wrap32
│   └── zx.id               zx8, zx16, zx32
├── mem/
│   ├── raw/                memcpy / memmove / memset / memcmp
│   │   ├── copy.id         c_memcpy, cpy_byte, cpy_prev
│   │   ├── move.id         c_memmove, mv_back, c_memcmp
│   │   └── fill.id         c_memset, set_byte, cmp_byte
│   ├── str/                the str* family
│   │   ├── len.id          c_strlen, c_strcpy, s_more
│   │   ├── cmp.id          c_strcmp, c_strncmp, ncmp_more
│   │   └── chr.id          c_strchr, chr_hit
│   └── heap/               kmalloc / kfree, a real free-list allocator
│       ├── blk/            the block header and the region
│       │   ├── init.id     heap_init, heap_end, align8
│       │   ├── hdr.id      bsize, bused, bset
│       │   └── link.id     bsetprev, init_rest, fix_prev
│       ├── get/            allocation
│       │   ├── new.id      kmalloc, blk_need, kzalloc
│       │   ├── find.id     find_fit, fit_at, walk_next
│       │   └── take.id     alloc_at, split_blk, re_min
│       └── put/            freeing, coalescing, resizing
│           ├── free.id     kfree, free_at, merge_blk
│           ├── coal.id     coal_next, coal_prev, re_len
│           └── re.id       krealloc, re_grow, re_move
└── fmt/                    printk / printf
    ├── conv/               one value, or one field, into text
    │   ├── base.id         str_of_base, digit_ch, nz_str
    │   ├── pad.id          pad_out, pad_str, pad_n
    │   └── txt/
    │       ├── rep.id      rep_ch, zero_pad, tail1
    │       ├── list.id     lsetw, lgetw, wzero
    │       └── ch.id       ch_of, str_at, ptr_str
    ├── spec/               parsing one % specifier
    │   ├── core.id         fmt_spec, fmt_conv, adv_nz
    │   ├── flag.id         scan_flags, is_flag, take_flag
    │   └── more/
    │       ├── set.id      set_flag, scan_width, is_digit
    │       ├── len.id      take_digit, scan_len, is_lenc
    │       └── code.id     take_len, len_code, hh_code
    └── emit/               dispatch on the conversion, and output
        ├── go.id           c_format, fmt_step, fmt_lit
        ├── pick/
        │   ├── num.id      conv_do, conv_num, conv_uns
        │   ├── text.id     conv_cs, conv_sp, conv_p
        │   └── rest.id     spec_txt, next_arg, c_puts
        └── val/
            ├── sgn.id      dec_of, sdec, narrow_s
            ├── uns.id      uns_of, narrow_u, narrow_u2
            └── mix/
                ├── pick.id narrow_s2, base_of, up_of
                └── out.id  c_printk
```

104 functions — 8 in `int/`, 44 in `mem/` (17 for the memory and string
routines, 27 for the allocator) and 52 in `fmt/`. The tree is deep because
the rule of 3 makes it deep: a
directory holds 9 functions, and a subtree of depth *L* holds 9 × 3ᴸ. The
same shape is what `c2id` will generate (`docs/DESIGN.md` §3).

---

## 1. `int/` — integer width semantics

C's integer types are all `word`s carrying a narrower value, so every
conversion C performs implicitly has to be an explicit call here. `c2id`
emits these wherever C's usual arithmetic conversions, an assignment to a
narrower type, or a cast demands one.

| function | contract |
| --- | --- |
| `sx_bits(word v, int bits) -> word` | sign-extend the low `bits` bits of `v` to 64. `bits` must be 1..64. |
| `sx8(word v) -> word` | sign-extend from 8 bits. `sx8(0xff)` = −1, `sx8(0x7f)` = 127, `sx8(0x80)` = −128. |
| `sx16(word v) -> word` | sign-extend from 16. `sx16(0xffff)` = −1, `sx16(0x8000)` = −32768. |
| `sx32(word v) -> word` | sign-extend from 32. `sx32(0x80000000)` = −2147483648, `sx32(0xffffffff)` = −1. |
| `wrap32(word v) -> word` | the result of C `int` arithmetic: keep the low 32 bits, read them signed. Identical in effect to `sx32`; separate because the call sites mean different things and `c2id` emits one per `int` operation. |
| `zx8(word v) -> word` | truncate to 8 bits, read unsigned. `zx8(0xff)` = 255, `zx8(0 - 1)` = 255. |
| `zx16(word v) -> word` | truncate to 16. `zx16(0 - 1)` = 65535. |
| `zx32(word v) -> word` | truncate to 32. `zx32(0 - 1)` = 4294967295. |

## 2. `mem/raw/`, `mem/str/` — memory and strings

Addresses are store addresses. Every access is bounds-checked by the store,
so a wrong length aborts rather than corrupting.

| function | contract |
| --- | --- |
| `c_memcpy(word dst, word src, word n) -> word` | copy `n` bytes forward; returns `dst`. Regions must not overlap. |
| `c_memmove(word dst, word src, word n) -> word` | copy `n` bytes correctly even when the regions overlap, in either direction; returns `dst`. Descending copy when `dst > src`, ascending otherwise. |
| `c_memset(word dst, word v, word n) -> word` | set `n` bytes to the low byte of `v`; returns `dst`. |
| `c_memcmp(word a, word b, word n) -> word` | compare `n` bytes as **unsigned**; negative / 0 / positive. The value is the difference of the first differing pair of bytes, which is what glibc returns. |
| `c_strlen(word s) -> word` | bytes before the NUL. |
| `c_strcpy(word dst, word src) -> word` | copy including the NUL; returns `dst`. |
| `c_strcmp(word a, word b) -> word` | unsigned byte compare to the first difference or the NUL; sign as C. |
| `c_strncmp(word a, word b, word n) -> word` | as `c_strcmp` but at most `n` bytes; 0 when `n` is 0. |
| `c_strchr(word s, int c) -> word` | address of the first byte equal to `(char)c`, or 0. `c` = 0 finds the terminator, as C requires. |

Internal helpers: `cpy_byte` / `cpy_prev` / `set_byte` copy or fill one byte
and return the next index (which is how a two-statement loop body fits in one
action); `cmp_byte` is the unsigned byte difference; `s_more` is the shared
"index `i` is still inside the string, including its NUL" predicate;
`ncmp_more` and `chr_hit` are the loop conditions of `c_strncmp` and
`c_strchr`.

## 3. `mem/heap/` — `kmalloc` / `kfree`

The store's `alloc` is an arena and never frees. Kernel code allocates and
frees constantly, so `crt` implements a real allocator **inside** one region
obtained from `alloc`, in `id`.

### Region and block layout

`heap_init(bytes)` takes `align8(bytes) + 16` bytes from the store and
exports the base as `crt_heap`:

```
crt_heap + 0      unused; keeps the first block header 16-byte aligned
crt_heap + 8      end address, one past the last block
crt_heap + 16     first block header
```

Every block is contiguous with its neighbours — an *implicit* list, walked by
address — and carries a 16-byte header:

```
h + 0     total size of the block in bytes, header included
          sizes are always a multiple of 8, so bit 0 is free and
          holds the in-use flag: 1 = allocated, 0 = free
h + 8     address of the previous block in address order, or 0
h + 16    payload, 8-byte aligned  <- this is what kmalloc returns
```

The previous-block link is what makes coalescing backwards O(1); without it a
free would have to rescan the heap from the start. The minimum block is 32
bytes (16 header + 16 payload), so a split never produces a block too small
to be linked, and a remainder below 32 bytes is left attached to the
allocation instead of being lost.

Search is **first fit** over the implicit list. Freeing merges with the next
block and then with the previous one; merging in only one direction leaves the
heap a staircase of fragments that never recovers.

| function | contract |
| --- | --- |
| `heap_init(word bytes)` | take a region of `align8(bytes)` usable bytes from the store and lay it out as one free block. Must be called before any other heap function. Calling it again abandons the old region (the store never reclaims it). |
| `kmalloc(word n) -> word` | address of `n` usable bytes, 8-byte aligned, or **0** if the heap cannot satisfy it. Contents are undefined. |
| `kfree(word p)` | return the block whose payload is `p` to the free list, coalescing with free neighbours on both sides. `p` = 0 is a no-op. `p` must be a live `kmalloc`/`kzalloc`/`krealloc` result. |
| `kzalloc(word n) -> word` | `kmalloc` plus zeroing, or 0. Real work: a reused block is not zero. |
| `krealloc(word p, word n) -> word` | resize. `n` = 0 frees `p` and returns 0; `p` = 0 allocates. Otherwise allocates a fresh block, copies `min(old payload, n)` bytes, frees `p`, and returns the new address — or returns 0 leaving `p` untouched if the allocation fails. There is no in-place grow. |
| `heap_end() -> word` | one past the last block. Nothing at or above it is heap. |
| `align8(word n) -> word` | round up to a multiple of 8. |

Internal helpers: `bsize` / `bused` / `bset` / `bsetprev` are the header
accessors; `init_rest` publishes a free block and repairs its successor's back
link, `fix_prev` does so only when a successor exists; `find_fit` / `fit_at` /
`walk_next` are the first-fit scan; `alloc_at` / `split_blk` hand a block out;
`free_at` / `coal_next` / `coal_prev` / `merge_blk` are the free path;
`blk_need`, `re_grow`, `re_move`, `re_len`, `re_min` support the API above.

The heap is not thread-safe or reentrant; `id` has neither threads nor
signals, so there is nothing to be safe against.

## 4. `fmt/` — `printf` / `printk`

| function | contract |
| --- | --- |
| `c_format(word fmt, word[] args) -> string` | format `fmt` (a store address of a NUL-terminated C format string) with `args`, and return the result as an `id` string. |
| `c_printk(word fmt, word[] args)` | `c_format`, written to stdout and flushed, with **no** newline appended — the format string says whether there is one, as with C's `printf`. |
| `c_puts(word s)` | the string at `s`, then a newline — C's `puts`. |
| `str_of_base(word v, int base, int upper) -> string` | `v` as an **unsigned** integer in `base` (2..16); `upper` selects `A`–`F`. Correct above 2⁶³, because it divides with `udiv`/`umod` rather than the signed `/` and `%`. |

`id` has no varargs, so the arguments arrive as a `word[]`. **Every element of
that list must already be `word`-typed** — see the comment on `wzero()`:
`idc.py` builds a list literal through a C varargs call whose cells are read
as `long long` and inserts no widening cast, so an `int` expression in a
`word[]` literal comes back with garbage in its top 32 bits. Write
`[wv(0 - 1), wv(42)]`, not `[wv(0 - 1), 42]`.

### Supported

* conversions `%d %i %u %x %X %o %c %s %p %%`
* length modifiers `hh` (8-bit), `h` (16), none (32, i.e. C `int`),
  `l`, `ll`, `z` (64)
* flags `-` (left justify) and `0` (zero pad)
* minimum field width, in decimal

The length modifier narrows the argument before it is printed, exactly as
C's `va_arg` would: `%d` of a 64-bit word prints its low 32 bits
sign-extended, `%x` of −1 prints `ffffffff`, `%hhd` of 255 prints `-1`.
Zero padding goes after a minus sign (`%05d` of −42 is `-0042`), the `0` flag
is ignored when `-` is present, and — matching glibc — the `0` flag does not
apply to `%s` or `%c`. `%s` of a null pointer is `(null)`; `%p` is `(nil)`
for null and `0x`-prefixed lowercase hex otherwise.

### Not supported, and visible when used

An unrecognised specifier is **echoed literally**, from its `%` through the
character that ended it, so a gap shows up in the output instead of silently
disappearing. This covers precision (`%.3s`), `*` widths, the flags `+`,
`' '` and `#`, the length modifiers `j`, `t`, `L`, `q`, and any unknown
conversion letter.

Internal structure: `c_format` walks the format string with a two-element
cursor (`st[0]` byte offset, `st[1]` next argument); `fmt_spec` allocates a
five-element parse state per specifier (`fs[0]` the offset of the `%`,
`fs[1]` `-`, `fs[2]` `0`, `fs[3]` width, `fs[4]` length code 0/1/2/3);
`scan_flags` / `scan_width` / `scan_len` fill it in; `conv_do` → `conv_num` →
`conv_uns` → `conv_cs` → `conv_sp` → `conv_p` → `spec_txt` is the dispatch
chain, lazy rather than eager so that an arm which is not taken does not
consume an argument. `narrow_s` / `narrow_u` apply the length modifier;
`pad_out` / `pad_str` apply the width and flags; `lsetw` / `lgetw` are the
list accessors `id` requires for a list that arrived as a parameter.

## 5. Known divergences from C

Stated plainly, because a port claim is only as good as its gaps.

* `%c` of 0 produces nothing. An `id` string is NUL-terminated and cannot
  carry an embedded NUL; C's `printf("%c", 0)` writes one byte.
* Unsupported specifiers are echoed. glibc agrees for unknown conversion
  letters (`%y`, `%R`, `%5y` all come back verbatim) but not for its own
  extensions — glibc silently accepts `q`, `Z` and `B`, and drops a length
  modifier attached to an unknown letter. `crt` echoes all of them.
* `c_memcmp` / `c_strcmp` / `c_strncmp` return the byte difference. C
  promises only the sign. This matches glibc as built by `cc -fno-builtin`;
  at higher optimisation levels the compiler folds comparisons of string
  literals into ±1 instead, so a differential test must disable builtins to
  compare exact values rather than signs.
* `heap_init` cannot give the region back to the store, which has no `free`.
* No floating point: `%f`, `%e`, `%g` are unsupported and echo.
