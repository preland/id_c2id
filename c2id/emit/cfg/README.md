# `emit/cfg/` — the control flow graph

Turns a parsed C function body into a list of **basic blocks**: a block is a
list of straight-line statement node ids plus one terminator. This is the
stage that makes `docs/DESIGN.md` §2 work — every C control construct becomes
an edge here, so the block emitter downstream never sees `while`, `switch`,
`break` or `goto` at all, only "statements, then go to block N".

Specified by `docs/EMITTER.md` §2; the shape of the output it feeds is
`tests/lowering/`.

## Interface

Seven functions. Everything else in this directory is machinery.

| function | meaning |
| --- | --- |
| `cfg_build(id)` → `int` | build the graph for body statement node `id`; returns the entry block, always `0` |
| `cfg_nblocks()` → `int` | how many blocks |
| `cfg_stmts(blk)` → `int[]` | the straight-line statement nodes of block `blk`, in order |
| `cfg_term(blk)` → `string` | `"jmp"`, `"cond"` or `"end"` |
| `cfg_succ(blk)` → `int` | the target of a `"jmp"`, or the **true** edge of a `"cond"` |
| `cfg_alt(blk)` → `int` | the **false** edge of a `"cond"`; `-1` otherwise |
| `cfg_expr(blk)` → `int` | a `"cond"`'s condition, an `"end"`'s returned value (`-1` for `return;`); `-1` for a `"jmp"` |

The three terminators map onto the generated `id` exactly as
`tests/lowering/` does it:

```
"jmp"   blkN(word fp) { ...statements... } return word SUCC;
"cond"  blkN(word fp) { word nxt = ALT; if(COND) { nxt = SUCC; } } return word nxt;
"end"   blkN(word fp) { poke64(fp, EXPR); } return word 0 - 1;
```

`cfg_build` resets the graph, so the emitter builds and consumes one C
function at a time — the same contract `emit/fr/` has for frame layout. Block
numbers are dense and start at 0, so `cfg_nblocks()` is also the size of the
dispatch tree.

Every statement node id handed back is a real node in the parser's store
(`NOTES.md` §4), so the emitter reads it with `k_of`/`a_of`/`s_of` as usual.
Two node kinds are *synthesised* here rather than parsed: the `expr` node that
wraps a `for` step clause, and the `bin ==` node a `switch` dispatch block
branches on. Both are ordinary nodes; nothing downstream needs a special case.

## How each construct lowers

`cur` is the block being filled when the construct starts; filling continues
in the last block named.

| C | blocks and edges |
| --- | --- |
| `expr;` `decl;` | appended to `cur` |
| `if (c) A else B` | `cur` → cond `c` → then / else; both ends jump to a join |
| `while (c) B` | `cur` → cond; cond → body / exit; body end → cond. break → exit, continue → cond |
| `do B while (c)` | `cur` → body; body end → cond; cond → body / exit. break → exit, continue → cond |
| `for (i; c; s) B` | `i` in `cur`; `cur` → cond; cond → body / exit; body end → **step**; step (holding `s`) → cond. break → exit, continue → **step** |
| `switch (v) B` | a chain of blocks testing `v == case value`, each falling to the next, the last falling to `default` or to the exit; each `case` starts a block, and the previous case simply falls into it. break → exit |
| `break` / `continue` | unconditional edge to the top of the relevant stack |
| `goto L` | unconditional edge to L's block; forward gotos are fixed up at the end |
| `L:` | starts a block, recorded in the label table |
| `return e` | terminator `"end"` with `e` (or `-1`) |

Blocks with no statements are normal and expected: a loop's condition block
holds only its terminator, and an `if` with no `else` still gets an empty else
block that jumps to the join. Keeping them removes every special case from the
builder; folding them away is a peephole pass on the finished graph, and the
graph is the right place to do it because by then no C syntax is involved.

**`continue` in a `for` runs the step clause.** That is the one place where a
mechanical translation of C is silently wrong — send `continue` to the
condition and the loop counter stops advancing. It is why the break and
continue targets are two independent stacks rather than one: a `switch` pushes
only a break target, so `continue` inside a `switch` inside a loop still
reaches the loop's step block. `tests/c/030_control.c` is the differential
test.

**Code after a jump gets its own block.** `return`, `goto`, `break` and
`continue` all open a fresh block immediately, which keeps the invariant that
*the block being filled is never already terminated* — and unreachable code
still needs somewhere to live, because it may contain a label that something
jumps to:

```c
goto tail;
n = 5;          // unreachable, but it is in a block that falls into `tail:`
tail:
```

## Layout

```
cfg/
  st/                 state and primitives
    var/              the globals, and creating a block
      g.id g2.id      cfg_init and the exported lists
      mk/             new_block, the current block, add_stmt, top_of
    acc/              reading and writing a block's row
      get.id get2.id  the six accessors above
      set/            set_kind/set_sa/set_sb/set_ex, term_jmp/term_cond/term_end
    tab/
      stk/            the break and continue stacks, loop_close
      lab/            the label table, the goto fix-up list, the case table
      run/            case_add/drop_cases, and cfg_build itself
  w/                  the statement walker: the dispatch chain, and the jumps
    s/                the middle of the chain
    j/                break, continue, return, goto, label, case
  ct/                 the constructs that build subgraphs
    a/                if, and while / do-while
    b/                for
    c/                switch and its dispatch chain
```

## State

All of it is exported lists, because `id` has no records and a block is
therefore one row across several of them. `cfg_init()` re-runs the `export`
statements, which rebinds each global to a fresh empty list — that is the
reset, and it is why there is no separate clearing chain.

| global | type | meaning |
| --- | --- | --- |
| `bstmt` | `int[][]` | statement node ids, per block |
| `bkind` | `string[]` | terminator kind, per block |
| `bsa`, `bsb` | `int[]` | successor A and B, per block |
| `bex` | `int[]` | terminator expression node, per block |
| `curb` | `int[]` | one cell: the block being filled |
| `btgt`, `ctgt` | `int[]` | break and continue target stacks |
| `lbls`, `labb` | `string[]`, `int[]` | label name → block |
| `fixs`, `fixb` | `string[]`, `int[]` | unresolved `goto`: label name, and the block whose edge needs it |
| `casv`, `casb` | `int[]` | case value node (`-1` = `default`) → block, for the switch being walked |

New local names introduced by this module, for `NOTES.md` §3's registry — a
name has one type program-wide, so these are reserved: `blk`, `nxt`, `alt`,
`stp` (`int`, block numbers) and `mark` (`int`, a case-table watermark).

## Not done here

* **Short-circuit `&&`, `||`, `?:`.** `docs/EMITTER.md` §3 requires these to
  become extra blocks with a frame temporary, and says so as the one place
  expression emission reaches back into this builder. A condition is currently
  handed to the emitter as a single expression node, which is correct only
  while the right-hand side has no side effects.
* **A `switch` subject with side effects.** The dispatch chain tests
  `subject == value` in each block, as `docs/EMITTER.md` §2 describes, so a
  subject like `switch (f())` is evaluated once per test instead of once. The
  fix is a frame temporary, and it should land together with the one for
  short-circuit operators rather than inventing a second mechanism.
* **Dead-block elimination.** Unreachable blocks (the one opened after a
  `return`, an empty `else`) are left in the graph.
* **Diagnostics.** `break` outside a loop, or a `goto` naming a label that
  does not exist, leave an edge to block `-1` rather than reporting an error;
  the front end is expected to have rejected that C.
