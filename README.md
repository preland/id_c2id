# c2id — a C→`id` compiler, written in `id`

`c2id` translates preprocessed C into [`id`](../idc), a deliberately tiny
language that allows 3 actions per block, 2 levels of nesting, 3 functions per
file, and has no structs, no pointers, and no
bitwise operators. `c2id` is not a translation exercise bolted onto `id` from
the outside — it is itself written in `id`, obeying every one of those rules
with no exemptions.

This repository was extracted from `linux_id`, a long-horizon port of the
Linux kernel to `id`. `linux_id` is `c2id`'s first real client and now
reaches this repository at `../id_development/c2id` rather than containing it; the
kernel tree, the corpus of kernel files, and the tools that measure progress
against them live there, not here.

## The stdin/stdout interface

`id` has no filesystem builtins, so `c2id` cannot open its input or write its
output files directly. Instead it reads preprocessed C on stdin and writes
the whole output project to stdout as a stream of tagged files:

```
==== FILE b/0/0/0/0/0/f0.id
main(int argc, string[] argv) {
...
==== FILE b/0/0/0/0/1/f0.id
...
```

`tools/c2id.sh` is the thin driver that runs the compiled `c2id` binary,
splits that stream into real files, and copies `crt/` alongside the result so
the generated project has its runtime. This is the same bargain `id` itself
makes with `bin/idc`: the language has no filesystem or subprocess builtins,
so a non-`id` driver layer for input/output is legitimate and expected —
everything else stays in `id`.

## The design, in short

C is not mapped onto `id`'s type system. Instead the C abstract machine is
modelled directly: a flat, bounds-checked byte memory plus a 64-bit `word`.

* **Structs become offsets, pointers become integers.** `p->field` is
  `peek32(p + OFF_field)`; `&x` is a frame address plus a compile-time offset;
  a `struct` costs nothing at translation time beyond the offsets it hands
  out.
* **One `id` function per basic block.** Each C function compiles to a
  control flow graph; each block becomes a function taking a frame pointer
  and returning the number of the next block, and a dispatch loop drives it.
  `goto`, `break`, `continue`, `switch`, and every loop form are then just
  edges in the graph — no special-casing in the emitter, and nesting never
  exceeds 2 because a basic block is straight-line code.
* **No exemptions from `id`'s rules.** The generated project obeys the same
  3-action, 2-nesting, 3-function, one-name-one-type, and
  duplicate-logic rules as hand-written `id`. A block over 3 actions is split
  into a chain of ordinary functions; the project layout scheme fills
  directories three files at a time automatically.
* **Memory-safe by construction.** Every load and store goes through a
  bounds-checked primitive (`peek8/16/32/64`, `poke8/16/32/64`), so an
  out-of-bounds access traps instead of corrupting memory.

Full architecture: `docs/DESIGN.md`. The emitter's exact contract, block by
block: `docs/EMITTER.md`. The ABI every part of `c2id` is written against —
its directory map, its name→type registry, its node layout: `c2id/NOTES.md`.

## Layout

| path | what it is |
| --- | --- |
| `docs/DESIGN.md` | the architecture — read this first |
| `docs/EMITTER.md` | the emitter's spec: frames, CFG construction, calls, project layout |
| `docs/ID_EXTENSIONS.md` | what `id` itself had to gain, and why each addition is unavoidable |
| `docs/ID_CHEATSHEET.md` | how to write `id` without relearning it the slow way |
| `docs/SELFHOST.md` | making `bin/idc` the compiler and retiring `idc.py` (since deleted) |
| `docs/STATUS.md` | the honest ledger: what works, what does not, and the command that shows each |
| `c2id/` | the C→`id` compiler itself, written in `id`: `front/` (`lex`, `parse`, `fold`), `mid/` (`hook`, `lower`, `pass`), `back/` (`emit`, `ind`, `link`) |
| `c2id/NOTES.md` | the ABI: directory map, name→type registry, AST node layout |
| `crt/` | C-semantics runtime (integer widths, memory, string/format helpers), in `id` |
| `rt/` | the libc a translated program links against (`libc/`, `ext/`: printf family, ctype, stdio, builtins), in `id` |
| `include/` | the only system header path `c2id.sh` gives the preprocessor |
| `tools/` | thin bash drivers: `c2id.sh`, `check.sh`, `idprobe.sh` |
| `tests/` | the differential cases (`tests/c/`), the front-end harness (`tests/frontend/`), and the lowering reference (`tests/lowering/`) |

## Running it

`c2id` builds and runs alongside the `id` toolchain, which lives in the
sibling `idc/` submodule of the `id_development` umbrella repository — from
inside this checkout, that's `../idc/bin/idc`.

```sh
tools/c2id.sh <in.c|in.i> <outdir>   # compile one translation unit
../idc/bin/idc c2id                  # build the translator itself
tools/check.sh                       # everything to run before committing
```

`idc c2id` runs under the default test requirement: every function in `c2id/`
carries two passing cases (`docs/TESTS.md` in the umbrella), so a build that
exits 0 means they all ran. Run it with `IDSTD_HOME` set to the idstd you mean
to build against; `c2id` calls idstd (`lget`, `str_slice`, `lst_str_insert`, ...)
rather than keeping its own copies, and `idc` rejects a function whose logic
matches an idstd one, so an idstd change can break this build in either
direction. A call that quietly changes meaning when idstd does is what the cases
exist to catch: `str_slice` went from (start, end) to (start, count), and three
of `c2id`'s calls to it were wrong until cases ran them.

`tools/check.sh` runs `idc --check` (id's structural rules over `c2id/` and
`crt/`), `tools/idprobe.sh` (is there a usable `id` toolchain with the
systems extensions this needs — `word`, `alloc`, `peek*`/`poke*`?), and
`tests/run.sh` (the differential harness: compile with `cc`, translate with
`c2id`, build with `idc`, and require the two to agree byte for byte). Each
step passes, fails, or is skipped with a stated reason; a skip is never
folded into the pass count.

## Status

See `docs/STATUS.md` for exactly which stages build and pass tests, and which
do not and why — a row there says "works" only if a command shows it.

C programs of real size translate, build with `idc` and run: the corpus in
`docs/STATUS.md` (inih, the libmd digests, xxhash, lz4, miniz's inflate, and the
programs in `tests/c/`) matches `gcc -O2` on stdout and exit status. What c2id
cannot translate correctly it refuses, at translate time, with `file:line` and the
construct's name, and writes nothing: `long double` (`float` and `double` are modelled; cJSON needs only those), compound
literals, a struct passed through `...`, an `fopen` mode outside r, w and a with b and `+`, the address of a variadic runtime function, signals, termios, 128-bit integers, `packed` structs, and
every called function that neither the program nor the runtime provides, all
listed at once. `tests/run.sh` has a case for each (`tests/c/9*.c`, with a
`.refuse` file naming the lines the refusal must contain). The translation is also slow, by the nature of the design (one flat
bounds-checked store, one dispatch loop per call): `docs/STATUS.md` has the
measured ratios against `gcc -O2`.

`c2id` itself obeys every `id` rule with no exemption, test cases included, and
is built with `bin/idc` everywhere. See the umbrella's `docs/GAPS.md`
B6/B7/B8 for the rules `idc.py` never enforced.
