# Picking this up

Written for whoever works on this next, including future me. It says what is
done, what the next move is, and — more usefully — which mistakes are
already paid for.

## Start here

```sh
tools/check.sh             # everything you should run before committing
```

`tools/check.sh` runs `tools/lint3.sh`, `tools/idprobe.sh`, and
`tests/run.sh`, and skips — loudly, with a reason — whatever the machine
cannot do. Add `--require-all` on a machine that is supposed to be able to do
everything.

Read `docs/DESIGN.md`, then `docs/EMITTER.md`. `c2id/NOTES.md` is the ABI and
is binding. `docs/ID_CHEATSHEET.md` is how to write `id` without relearning it
the slow way.

## What is actually done

* **`id` itself** now has what systems code needs: a 64-bit `word`, bitwise
  operators, hex literals, and a flat bounds-checked store. Committed in
  `../idc` on branch `kernel-port/systems-extensions`, 116 tests passing. See
  `docs/ID_EXTENSIONS.md` for why each addition was unavoidable and what was
  deliberately left out.
* **The lowering is proven.** `tests/lowering/` is a hand-written `id` project
  in exactly the shape the emitter must produce, and it compiles under
  `idc.py` — which enforces every rule — and computes the right answer. That
  was the one genuinely risky idea here, and it holds.
* **The harness compares against `cc`**, so no claim can outrun its evidence.
* **The front end**: the C lexer, the AST store, the declaration/type parser,
  the expression parser, the statement parser, frame layout, and the project
  layout scheme. All compile and run as one program (`tests/frontend/`) and
  parse real C statements correctly. Currently one known failure — see
  `docs/STATUS.md`'s "Known gaps in what is built": `c2id`'s own `lset`
  collides with `idstd`'s when the front end is built alongside the
  standard library, so `tests/run.sh` case `frontend-integration` fails.

`docs/STATUS.md` is the ledger and is kept honest.

## The next move

Finish `c2id` in the order in `docs/EMITTER.md` §7 — frame layout, then
straight-line expression emission, then the CFG, then calls, then pointers
and structs. Each step should turn one row of `docs/STATUS.md` green by
making one file in `tests/c/` pass, not by being written.

The first real milestone is `tests/c/010_arith.c` end to end.

Before any of that, the `lset` collision in `docs/STATUS.md` is worth
clearing: `c2id/lex/drv/st/store.id` should call `idstd`'s `lset` instead of
defining its own, the way `id_development`'s own `compiler/lex` and
`compiler/parse` already do (see `idc/tests/idstd_expect.txt`). It is a small
fix and it currently fails `tools/check.sh` on every run.

## Things already learned the hard way

* **`bin/idc` is the compiler; `idc.py` is stage-0 bootstrap only.** That is
  the point of the language — `id` is constrained so that a function has
  essentially one viable expression, and a self-hosted compiler written in it
  is far easier to read, trace and debug than a 3,400-line Python monofile.
  Anything that pushes work back into `idc.py` is moving the wrong way.

  While the self-hosted checks are being written (`docs/SELFHOST.md`), a
  program can still compile under `bin/idc` while violating rules `idc.py`
  would catch. Until that list is empty, cross-check with `idc.py` when a
  structural rule is in doubt — but treat every such case as a **bug in the
  self-hosted compiler to be fixed**, not as a reason to switch compilers.
* **`(import xs)[i] = v` silently does nothing.** It parses as a comparison.
  Store through `lset`/`sset` — but check `idstd` first; it may already define
  the name you are about to add. See the collision recorded in
  `docs/STATUS.md`.
* **One name, one type, program-wide.** This is the constraint that bites at
  scale: a parameter named `t` in one file conflicts with a `t` four
  directories away. `c2id/NOTES.md` §3 is the registry — keep it current or
  you will thrash. `lset` and `sset` cannot share a parameter name; that is
  why they take `xs` and `ss`.
* **Two functions with the same logic are a compile error**, compared up to
  renaming of their own locals. Two constants returning the same literal
  collide. Give each magic number one name and call it; turn "same but for Y"
  into a parameter. This applies to *generated* code too — which is why every
  emitted block function ends with its successor's number as a literal.
* **Plan the directory tree before writing.** Adding a level later means
  moving everything beneath it. A leaf directory holds 9 functions; depth *L*
  holds 9 × 3^L. `.md` files are free and don't count.
* **`else if` costs an action**, so `if/else if/else` alone fills a block.
  Use the delegation chain instead: one decision per function.

## Where the honest difficulty is

Not in the parser, and not in the lowering — those are ordinary work now. It
is in these, roughly in order:

1. **Short-circuit evaluation.** `&&`, `||` and `?:` cannot be one expression
   when the right operand has side effects; they need extra blocks and a
   frame temporary. This is the one place expression emission has to reach
   back into the CFG builder, and it is easy to get subtly wrong.
2. **Integer width discipline.** Every load from a narrower signed type needs
   sign extension, every unsigned operation needs the `u*` builtin. Getting
   one wrong produces answers that are right for small inputs. The
   differential tests exist mainly for this.
3. **`continue` inside `for`** must run the step clause. Mechanical
   translations get this wrong.
4. **Inline assembly.** Untranslatable by construction. The plan is a table of
   hand-written `crt` intrinsics with anything unmatched reported as a counted
   gap — never silently mistranslated.
5. **Concurrency.** Translated code is single-threaded. Locks become no-ops,
   which is correct for single-threaded execution and wrong for anything
   else. It should stay flagged rather than quietly assumed.

## What "done" would even mean

Not a bootable kernel — `linux_id`, which consumes this compiler, needs
inline asm, MMIO, interrupts and a device model on top of whatever `c2id`
produces, none of which the flat-store model has by itself. What is genuinely
reachable here is **a C source file translated and provably matching `cc`'s
output**: the differential cases in `tests/c/`, growing toward real leaf
algorithms. That is a real result, it is testable at every step, and
`docs/STATUS.md` should only ever claim the part that a test demonstrates.
