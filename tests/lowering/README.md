# The lowering, by example

This is a hand-written `id` project in **exactly** the shape `c2id` must
generate. It is the emitter's specification: if the generated output does not
look like this, the emitter is wrong.

It exists because the lowering in `docs/DESIGN.md` §2 is the one genuinely
risky idea in this repository — everything else follows from it — and an idea
like that is worth proving by hand before building a compiler to produce it.

## The C being modelled

```c
int sum_to(int n) {
    int t = 0;
    for (int i = 1; i <= n; i++)
        t += i;
    return t;
}
```

## What it demonstrates

* **The frame.** `fp+0` return value, `fp+8` parameter `n`, `fp+16` local `t`,
  `fp+24` local `i`. Locals live in the flat store, not in `id` variables,
  because a local written in one block and read in another has nowhere else to
  live — `id` variables are function-private. This is also what gives `&x` a
  meaning for free.
* **One `id` function per basic block**, taking `word fp` and returning the
  number of the next block. `blk4` returns `0 - 1`, the end marker.
* **A `for` loop as four blocks** — init, condition, body, step — with no loop
  construct in the generated code at all. `while`, `do/while`, `break`,
  `continue`, `switch` and `goto` all lower to the same thing: an edge.
* **The dispatch tree** (`disp_sum_to` → `disp_lo`/`disp_hi` → …), a binary
  search over block numbers, each level a single `if`/`else` so that no block
  exceeds 3 actions.
* **The driver loop**, which runs blocks until one returns the end marker.
  Blocks return here rather than tail-calling each other, so a loop of any
  length runs in **constant stack** — `id` guarantees no tail calls.
* **Every `id` rule satisfied without exemption**: at most 3 actions per
  block, nesting never deeper than 2, 3 functions per file, 3 entries per
  directory. Each block function ends with a distinct block-number literal,
  which is also what keeps them distinct under `id`'s duplicate-logic rule.

## Verify it

```sh
../../../idc/bin/idc . -o /tmp/sumto && /tmp/sumto   # 55
```

`bin/idc` enforces the action limit, the nesting limit, the one-name-one-type
rule and the duplicate-logic rule, and two rules `idc.py` never did: a call is
never an argument to a call, and a return clause is a name or a literal. So
every value that takes a step is named first -- `word t0 = peek32(fp + 24);`
-- and the temporaries are `word`s named `t0`, `t1`, ... the way the emitter
names them. Passing under `bin/idc` is the claim being made here.
