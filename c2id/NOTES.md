# c2id — the ABI

This file is the contract every part of `c2id` is written against. It is a
`.md`, so it costs nothing against the 3-entries-per-directory rule, and it
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

Every directory holds at most 3 entries; `.md` files are free. Leaf
directories hold 3 `.id` files × 3 functions = 9 functions.

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

Every directory above holds at most 3 entries — count them. `main` lives in
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
  `file:line: error:` diagnostics; `idc.py` is bootstrap-only and there is no
  fallback.
