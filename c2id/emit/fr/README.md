# Frame layout

Where each C parameter and local lives inside its function's frame in the flat
store.

C locals cannot be `id` variables. An `id` variable is private to one function,
and after the CFG split a local written in block 3 is read in block 9 — so it
has nowhere else to live. Two things fall out for free: `&x` becomes ordinary
arithmetic (`fp + OFF`), and recursion works, because each call allocates its
own frame.

`fp + 0` is always the return-value slot, which is why the running size starts
at 8.

## Interface

| function | meaning |
| --- | --- |
| `frame_reset()` | start a new function |
| `frame_add(name, t)` → `int` | give `name` a slot for type `t`; returns its offset |
| `frame_off(name)` → `int` | offset, or -1 if not a local (so the caller treats it as a global) |
| `frame_wid(name)` → `int` | byte width, which selects `peek8/16/32/64` |
| `frame_size()` → `int` | current size; round up with `align_up` for the allocation |
| `align_up(off, w)` → `int` | round `off` up to a multiple of `w` |

Depends on `type_size(t)` and `type_align(t)` from `../../parse/ty/`.

## Verified against `cc`

For `struct { long ret; char a; int b; char c; long d; }` — the frame shape of
a function with those locals — offsets and total size are identical to what
the C compiler produces:

```
        id      cc
a       8       8
b       12      12
c       16      16
d       24      24
size    32      32
```

Matching C's layout exactly is not required for the frame to *work* — the
store is byte-addressed and would accept any packing — but it is what allows a
translated struct to be compared against the real one, which is how the
struct tests will be written.
