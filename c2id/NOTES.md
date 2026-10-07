# c2id — the ABI

This file is the contract every part of `c2id` is written against. It is a
`.md`, so it is not part of the program, and it
exists because in a language where a name has **one type program-wide** and
functions must be **provably distinct**, the registries below are not
documentation — they are the thing that keeps a 1000-function program from
thrashing.

Read `../docs/DESIGN.md` first for what `c2id` does. This file is *how*.

## 1. Pipeline

```
preprocessed C on stdin
   │
   ├─ lex/    → token stream in parallel lists (tkind, ttext, tline)
   ├─ parse/  → AST in parallel lists (nk, na, nb, ns, nt, nl, nm)
   └─ emit/   → id project text on stdout, as a stream of tagged files
```

Output format, split into real files by `tools/c2id.sh`:

```
==== FILE main.id
main(int argc, string[] argv) {
...
==== FILE blk/blk1.id
...
```

`id` has no filesystem builtins, so emitting a tagged stream and letting a
three-line driver split it is the same bargain `bin/idc` already makes.

## 2. Directory map

The tree below was laid out three entries to a directory (a since-dropped
rule; directories have no entry limit now). Leaf directories hold 3 `.id`
files × 3 functions = 9 functions.

```
c2id/
  NOTES.md                                 (free -- not a .id file)
  lex/        C lexer                      (budget ~70 functions)
    ch/         character classification
    tok/        token scanners
    drv/        load stdin, token store, cursor
  parse/      C parser                     (budget ~200 functions)
    ast/        node store + constructors + accessors
    ty/         types, declarators, typedefs, struct layout
    code/
      ex/       expressions
      st/       statements + declarations
  emit/       CFG + id emission            (budget ~200 functions)
    fr/         frame layout (locals → offsets)
    cfg/        basic blocks + edges
    gen/
      txt/      expression and block text
      out/      project-tree writer, and `main`
```

`main` lives in
`emit/gen/out/` because the program *is* its final stage: read, lex, parse,
emit.

## 3. Name → type registry

**A variable name has one type across the entire program.** Before inventing a
name, look here; after inventing one, add it here. Violations are a compile
error whose message points at an unrelated file four directories away.

| name | type | meaning |
| --- | --- | --- |
| `i`, `j`, `n`, `k` | `int` | loop counters, lengths |
| `c` | `int` | one character byte code |
| `pos` | `int[]` | the shared one-element cursor `[index]` |
| `src` | `string` | the whole input text |
| `s`, `out`, `txt` | `string` | text being built |
| `name` | `string` | an identifier's spelling |
| `kind` | `string` | a token kind or node kind |
| `id`, `a`, `b`, `ty` | `int` | AST node ids (`ty` = a type node id) |
| `ids`, `xs`, `l`, `m` | `int[]` | lists of node ids |
| `off`, `sz`, `w` | `int` | byte offset, size, width |
| `blk`, `nxt` | `int` | basic-block numbers |
| `ind` | `string` | indentation prefix |
| `ok`, `found`, `depth` | `int` | flags and counters |
| `xs` | `int[]` | an int list being stored into (`lset`) |
| `ss` | `string[]` | a string list being stored into (`sset`) |
| `tstr` | `string` | the node `nt` string field |
| `words` | `string[]` | a list of token spellings |
| `base` | `word` | store address of source byte 0 (lexer) |
| `radix` | `int` | numeric base of an integer literal |
| `inner` | `int` | the type a declarator is being applied to |
| `tk` | `int` | a token index |
| `msg` | `string` | the text of a refusal |
| `bad` | `int` | the offset of a format conversion that is not implemented, or -1 |
| `cch` | `int` | a character code in the check pass |
| `lit` | `string` | the raw text of a string literal node, quotes included |
| `fname` | `string` | the name of a base type in the check pass |
| `bt` | `int` | the type a pointer or array finally points at |
| `md` | `int` | the node of an `fopen` mode argument |

`xs`/`ss` and `ty`/`tstr` are split precisely because a name may have only one
type: `lset` and `sset` cannot share a parameter name, and the node ABI's `nt`
string field is spelled `tstr` so that a type node id can keep a short name.
Parameters count for this rule too, which is why `base` (a store address in the
lexer) had to become `inner` in the declarator parser.

Exported globals (reserved program-wide, reachable only via `import`):

| name | type | meaning |
| --- | --- | --- |
| `tkind`, `ttext` | `string[]` | token kind and spelling, per token |
| `tline` | `int[]` | source line, per token |
| `nk` | `string[]` | node kind, per node |
| `na`, `nb` | `int[]` | two int fields, per node |
| `ns`, `nt` | `string[]` | two string fields, per node |
| `nl`, `nm` | `int[][]` | two child-list fields, per node |
| `tynames`, `tykinds` | `string[]` | typedef name → type node id (as text) |
| `synames`, `syoff`, `sysz` | `string[]`, `int[]`, `int[]` | struct member table |
| `fnames`, `fnids` | `string[]`, `int[]` | function name → node id |
| `code` | `string[]` | accumulated output lines |
| `ntok` | `int[]` | per node: the index of the last token consumed when it was made |
| `tfile` | `string[]` | per token: the file the preprocessor says it came from |
| `curtk`, `dots`, `chk_flt` | `int[]` | one cell each: the token the parser has consumed up to; 1 after a `...` until its function type is made; 1 once the current function has had a floating-point use refused |
| `dgmsg` | `string[]` | the refusals, one `file:line: error: ...` each |
| `unres`, `unres_at` | `string[]`, `int[]` | functions called that nothing provides, and the token each was first named at |
| `nrnames`, `mxnames` | `string[]` | the functions that never return (exit, abort, _Exit and the `_Noreturn` ones, read from the tokens by `front/diag/noret/`), and every function that may reach one: a call to one of these ends its block, a statement calling one of the second is followed by a block boundary |
| `retty`, `fltys` | `int[]` | the return type of the function being emitted; whether the float and double type nodes exist yet (`[0]`) and their node ids (`[1]`, `[2]`) |

## 4. Node ABI

Every node is one row across the seven parallel lists. `newnode(kind, a, b,
s, t, l, m)` pushes one cell to each and returns the new id. Unused fields are
`0` / `""` / an empty list.

### Expressions

| kind | `na` | `nb` | `ns` | `nt` | `nl` | `nm` |
| --- | --- | --- | --- | --- | --- | --- |
| `num` | value | — | spelling | — | — | — |
| `str` | — | — | raw text | — | — | — |
| `var` | — | — | name | — | — | — |
| `bin` | left | right | operator | — | — | — |
| `un` | operand | — | operator | — | — | — |
| `post` | operand | — | `++` / `--` | — | — | — |
| `call` | callee | — | — | — | args | — |
| `idx` | base | index | — | — | — | — |
| `mem` | base | — | field | `.` / `->` | — | — |
| `cast` | operand | type node | — | — | — | — |
| `cond` | condition | then | — | — | `[else]` | — |
| `assign` | target | value | operator | — | — | — |
| `sizeof` | operand or 0 | type node | — | — | — | — |
| `stmtx` | — | — | — | — | statements | — |

`decl` nodes carry `nt` = `static` when the declaration's specifiers did (the
frame pass then makes the variable a global); `fn` type nodes carry `nb` = 1 when
the parameter list ended in `...`.

A function that returns a struct or union has a hidden first parameter: its `fn`
type node (`fn_node`, `front/parse/ty/dcl/d/sfx/more/sret.id`) lists a pointer to
the result type first and names it `sret_var` (`conf.id`), so every consumer of
the type (the wrapper, the `crt_callN` dispatch, `bind_params`) sees one more
parameter than the C source writes. A `call` node whose callee returns a struct
or union carries `nb` = 1 once the frame pass has put `&__tmpN` in front of its
arguments (`mid/pass/frame/sret.id`); `nb` is 0 on every other call.

### Statements

| kind | `na` | `nb` | `ns` | `nt` | `nl` | `nm` |
| --- | --- | --- | --- | --- | --- | --- |
| `expr` | expression | — | — | — | — | — |
| `decl` | init or -1 | type node | name | — | — | — |
| `if` | condition | — | — | — | then | else |
| `while` | condition | — | — | — | body | — |
| `dowhile` | condition | — | — | — | body | — |
| `for` | condition or -1 | step or -1 | — | — | init | body |
| `switch` | subject | — | — | — | body | — |
| `case` | value or -1 | — | — | — | — | — |
| `label` | — | — | name | — | — | — |
| `goto` | — | — | name | — | — | — |
| `break` | — | — | — | — | — | — |
| `continue` | — | — | — | — | — | — |
| `return` | value or -1 | — | — | — | — | — |
| `block` | — | — | — | — | statements | — |

### Types

| kind | `na` | `nb` | `ns` | `nt` | `nl` | `nm` |
| --- | --- | --- | --- | --- | --- | --- |
| `prim` | size in bytes | 1 if signed | name | — | — | — |
| `ptr` | pointee | — | — | — | — | — |
| `arr` | element | count or -1 | — | — | — | — |
| `fn` | return type | — | — | — | parameter types | — |
| `rec` | size | 1 if union | tag | — | member type ids | — |

`na` on `prim` is the byte width, so `sizeof` and pointer arithmetic never
need a second table.

## 5. Character codes

`id` has no character literals, so byte codes are written out. The ones this
program uses, kept here so they are typed once:

```
  9 tab      10 LF     13 CR     32 space   33 !    34 "    35 #    37 %
 38 &        39 '      40 (      41 )       42 *    43 +    44 ,   45 -
 46 .        47 /      48-57 0-9 58 :       59 ;    60 <    61 =   62 >
 63 ?        65-90 A-Z 91 [      92 \       93 ]    94 ^    95 _
 97-122 a-z 123 {     124 |     125 }      126 ~
```

## 6. Rules that bite, and the local answer

Learned from `../../id_development/demos` and its `BLOCKERS.md`:

* **`(import xs)[i] = v` silently does nothing.** It parses as a comparison
  expression statement. Always store through `lset(int[] xs, int i, int v)` —
  defined exactly once, in `lex/drv/st/store/append.id`, because the lexer needs it
  first. Same for `sset` (`string[]`, whose list parameter must be called `ss`,
  since one name may have only one type).
* **Only the declaring function may assign an exported scalar.** Any global
  counter is therefore a one-element list, not a scalar.
* **Two functions with identical logic are a compile error**, compared up to
  renaming of their own locals. Two constants with the same value collide.
  So: every magic number gets **one** named accessor and is called, never
  copied; "just like X but for Y" becomes a parameter, not a second function.
  For the emitter this matters most in the generated *output*: each emitted
  block function ends with a distinct block-number literal, which is what
  keeps the generated project legal under the same rule.
* **`else if` costs an action**, so `if/else if/else` alone fills a block.
  Prefer the delegation chain (`f → f2 → f3`), one decision per function.
* **A bare `[]` needs a known list type** — declare `int[] none = [];` first.
* **Evaluation order within one expression is unspecified.** Never mutate and
  read the same structure in one expression.
* **Build with `bin/idc`.** It enforces every rule above itself now, with
  `file:line: error:` diagnostics; there is no compiler other than `bin/idc` and no
  fallback.

## 7. Refusals

A wrong translation is worse than none, so what c2id cannot translate
correctly is refused at translate time with `file:line` and the construct's
name, and nothing is written. Refusals are collected rather than raised: the
parser records one (`front/diag/`: `diag_tok`, `diag_node`) and carries on, so
a run lists every one it can find. A syntax error is a refusal too
(`expression expected, found ...`, `file:line: error:` on stderr, exit 1, no
project written), and it also ends the parse (`diag_bail`: the cursor moves to
the end of the tokens, so no recovery loop can run on), since reading on after
one is how a translator spins. Three places make them:

* the parser, for what it sees as it reads, reached or not: a bitfield wider
  than its type
  (`front/diag/bits.id`), `vector_size`, `mode` and a `packed` that is on
  something other than a struct, union, enum or member (`front/diag/packed*.id`),
  `constructor`, `destructor` and `alias`, a zero-width or over-long bit-field in
  a packed record. What it cannot yet tell is needed is *deferred*
  (`front/diag/defer*.id`): the refusal is kept with the external declaration
  being read, and when that declaration ends it is kept for the function it
  defined (the check pass tells it, `mid/check/deferred.id`, only if the
  function is reached) or, if it defined none, told at once, except for the
  kinds that poison instead: a named address space (`__seg_gs`) or an array
  bound the folder cannot evaluate, in a declaration that defines no function,
  poisons the globals and types it declared (`sgv`, `sgt`; a record that holds a
  poisoned one by value is poisoned too, `seg_member`), and the check pass
  (`mid/check/seg*.id`) refuses a reached function that names a poisoned
  variable, takes a member of a poisoned record or its `sizeof`. The
  deferred kinds are the `typeof` that cannot be resolved, a `cleanup` of
  something other than a plain function name, a `__builtin_choose_expr`
  whose condition is not constant, a named address space and an unfoldable
  array bound inside a function;
* the check pass (`mid/check/`), over every function the program reaches with
  its locals bound: the address of a function the runtime provides that has no
  wrapper (`printf` and the variadic rest: `mid/check/rtval.id`; one with a wrapper
  is rewritten to it instead, `mid/check/wrap.id`) or of one the program defines with `...`
  (`mid/check/vaval.id`), a struct passed by
  value through the `...` of a variadic call (`chk_arg`), a printf-family
  format with a conversion the runtime does not implement, an `fopen` mode
  that is not r, w or a followed by b and `+` alone, a call refused by name (`refuse_names` in `conf.id`: termios,
  signals), and every called function that is neither defined in the program
  nor provided by the runtime, reported as one diagnostic with where each was
  first named; a C keyword read as a function (`unres_add`) is reported as a
  limit of the parser instead, not as a missing function. The runtime's names
  are `rt_names` in `conf.id`; keep it equal to what `rt/` and `crt/` define (a
  case in `tests/c/` for each name keeps it so);
  Constructs a header holds in functions nothing calls reach this pass as a
  `refuse` node (`front/diag/refuse.id`: an `asm` statement that does
  something, `_Generic`) or as a name (`regnames`: a variable bound to a
  machine register), and are refused here, `mid/check/extra.id`, only if the
  function is reached; a `typeof(expression)` is resolved here too, by the
  frame pass (`mid/pass/frame/tof/`), which has the types of the locals;
* nowhere else: once `diag_status()` is non-zero the driver (`unit/settle.id`)
  skips emission, `diag_report()` prints the lines, and `main` exits 1.

`tests/c/NAME.refuse` lists the lines a refused program's diagnostics must
contain.

## 8. Constructs the translator rewrites

* **Statement expressions** `({ ...; e; })`: a `stmtx` node (children in `nl`,
  in `scan_l`). The lowering (`mid/lower/val/stmtx*.id`) walks its statements
  into the graph where the expression is and stores the value of the last
  expression statement in a frame temporary.
* **Block scope of types**: typedef names, struct and union tags and
  enumerators end with the block that declared them (`ty_mark`/`ty_drop` around
  `parse_block`, `front/parse/ty/tab/reg/tb/scope.id`), and the innermost
  declaration of a name wins (`find_last`).
* **Variadic definitions**: see `docs/EMITTER.md`, "Variadic definitions".
* **Bit-fields**: see `docs/EMITTER.md`, "Bit-fields".
* **setjmp/longjmp**: see `docs/EMITTER.md`, "setjmp and longjmp".
* **A `switch` subject** that is more than a name or a number is evaluated once,
  into a frame temporary (`sw_subject`), not once per case test.
* **Atomic builtins** (`__atomic_*`, `__sync_*`): the frame pass rewrites the call
  once its arguments are bound (`mid/pass/frame/atom/`, tables `atom_*` in
  `conf.id`): the callee becomes a function of `rt/ext/builtin/atom*.id`, the order
  arguments are dropped, the operation code and the size of the object the first
  argument points at are added, and an answer that is a value of that object is cast
  to its type. A fence or a lock-free test becomes the constant.
* **The address of a runtime function**: a name used as a value that is a function
  the runtime provides is rewritten, in the check pass, to the function
  `__c2id_w_NAME` the header that declares it defines in C, which is marked reached
  (`mid/check/wrap.id`).
* **`cleanup`** (`front/parse/code/sb/cl/`): `T v __attribute__((cleanup(fn))) = e;`
  in a block makes the statements after it one nested block of their own, ending
  with `fn(p);`, where `p` is `__auto_type __cl_pN = &v;` declared just before (a
  name nothing else can take, so a shadowing declaration below cannot capture the
  call). A `return e;` in it becomes `{ __auto_type __cl_rN = e; fn(p); return
  __cl_rN; }`, a `return;`, `break` or `continue` that leaves the scope and a
  `goto` to a label outside it get `fn(p);` before them (`cl_scan`: `brk` and
  `cont` count the loops entered below the scope, mode 0 collects its labels).
  Two cleanups in one declaration nest, the second inside the first. A cleanup
  declared in a `for` initialiser is refused.
* **`packed` and `aligned`** (`front/parse/ty/dcl/r/pk/`): the attributes before
  the `{` and after the `}` of a struct or union are read before any member is
  placed (`rec_attr`: the node's `nt` is "packed"), every member then aligns to one
  byte (`mem_align`), a bit-field starts at the next bit (`bf_packed`), and
  `rec_aln` sets the alignment from `aligned(N)` (N exactly if packed, else the
  larger of N and the members'). A bit-field that spans its type's aligned unit is
  read through the bytes it spans, 1, 2, 4 or 8 of them
  (`back/emit/gen/txt/ex/a/bf/span.id`). A packed enum is the smallest integer
  type that holds its enumerators (`e/pk/`).
* **128-bit integers**: see `docs/EMITTER.md` "128-bit integers".
* **Which functions a unit emits**: from `main` and the initializers of its
  external variables; with no `main`, from every function that is not `static` or
  `inline` (`extern inline` included), those with a `used` attribute, and what
  they name. A `static` global nothing reached names is dropped, with its
  initializer (`greach`, `mid/pass/reach/more/glob.id`).
