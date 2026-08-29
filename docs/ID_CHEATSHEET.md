# Writing `id`: patterns that work

Distilled from `../../id_development/demos` (the id-written lexer, parser and
games). Every pattern here is copied from code that compiles. If you are
writing `id` for this repository, read this first — the language is small
enough that there is essentially one right way to express each construct, and
guessing wastes a lot of time.

## The rules, exactly

* **3 actions per block.** Every block: the function body, and the body of
  each `if`, `else`, `while`. One statement = 1. An `if` = 1, **and each
  chained `else` = 1 more** (so `if/else if/else` alone is 3). A `while` = 1.
  The `return` clause after the closing brace is **free** and may compute.
* **Nesting depth 2.** Function body is depth 0; `while { if { } }` is legal,
  one more level is not.
* **3 functions per file. 3 entries per directory** (`.id` files +
  subdirectories). **Non-`.id` files are free** — put as many `.md` notes as
  you like. So a leaf directory holds 9 functions; a tree of depth *L* holds
  9 × 3^L.
* **A name has one type across the whole program**, parameters included.
* **Variables are function-private** unless `export`ed; others read them with
  `(import name)`.
* **Two functions with the same signature and the same logic are an error**,
  compared up to renaming of their own parameters and locals. What
  distinguishes functions: operators, literals, and the names of called
  functions and imported globals.
* Declarations must initialise. No bare `int x;`.
* `//` comments only. No `for`, `break`, `continue`, `switch`, `+=`, `++`,
  ternary, or character literals.

Build with `bin/idc`. It enforces every rule above and reports
`file:line: error:` diagnostics; `idc.py` is bootstrap-only now and there is
no fallback. One thing worth knowing: if `bin/idc` says
`internal error: the self-hosted compiler emitted C that does not compile`,
the C compiler's own diagnostic is printed just above it — that is usually a
name colliding with a libc symbol (an exported global is emitted as a bare C
global, so `export int labs` collides with `labs()`).

## Function shape

```
name(int a, string s) {
  int x = a + 1;
} return int x;

accessor(int id) {
} return string (import nk)[id];      // empty body, all work in the return

side_effects_only() {
  print("hi");
} return void;
```

## Loops

```
walk(string src) {
  int i = 0;
  while(i < len(src)) {
    handle(charat(src, i));
    i = i + 1;
  }
} return void;
```

**Let the helper return the next index** when the body would need two actions:

```
scan(string src, int i) {
  while(i < len(src)) {
    i = scan_one(src, i);       // one action; scan_one returns i advanced
  }
} return void;
```

**Early exit** — there is no `break`. Use a flag in the condition, where the
flag doubles as the result:

```
find_ch(string src, int i, int c) {
  int found = 0 - 1;
  while(i < len(src) && found < 0) {
    found = probe(src, i, c);
    i = i + 1;
  }
} return int found;

probe(string src, int i, int c) {
  int r = 0 - 1;
  if(charat(src, i) == c) {
    r = i;
  }
} return int r;
```

**Accumulate** by threading the accumulator through a helper:

```
sum_list(int[] xs) {
  int i = 0;
  int total = 0;
  while(i < len(xs)) {
    total = total + xs[i];
    i = i + 1;
  }
} return int total;
```

## Dispatch (the replacement for `switch`)

**Lazy chain** — one decision per function, `else` hands off. Use when the
arms have side effects or recurse:

```
parse_stmt(int[] pos) {
  int node = 0;
  if(cur_text(pos) == "if") {
    node = parse_if(pos);
  } else {
    node = parse_stmt2(pos);
  }
} return int node;

parse_stmt2(int[] pos) {
  int node = 0;
  if(cur_text(pos) == "while") {
    node = parse_while(pos);
  } else {
    node = parse_stmt3(pos);
  }
} return int node;
```

**Eager chain** — 2 actions instead of 3, but the default is *always*
evaluated, so the default must be the safe one and the special case goes in
the `if`:

```
emit_expr(int id) {
  string s = ee2(id);
  if(k_of(id) == "num") {
    s = "" + a_of(id);
  }
} return string s;
```

**Independent guards** — when at most one fires and nothing is returned:

```
apply_key(int ev) {
  try_move(ev);
  try_turn(ev);
  try_quit(ev);
} return void;
```

**One big boolean** for a membership test — one action, however many terms:

```
is_kw(string w) {
  int ok = 0;
  if(w == "int" || w == "if" || w == "else" || w == "while" || w == "return") {
    ok = 1;
  }
} return int ok;
```

**A table** when it is really key → value:

```
dx_table() {
  int[] tbl = [0, 0, 0 - 2, 2];
} return int[] tbl;

dir_dx(int i) {
  int[] tbl = dx_table();
} return int tbl[i];
```

## Splitting a long routine

Thread the accumulated state forward through a chain; the last link's return
clause is the constructor call, which is free:

```
parse_func(int[] pos) {
  string name = cur_text(pos);
  advance(pos);
} return int func_sig(pos, name);

func_sig(int[] pos, string name) {
  int[] params = parse_params(pos);
  int[] body = parse_block(pos);
} return int func_ret(pos, name, params, body);

func_ret(int[] pos, string name, int[] params, int[] body) {
  advance(pos);
  string rt = parse_type(pos);
} return int node_func(name, rt, params, body);
```

Naming: `parse_x → x_step → x_done` for continuations, `f2 f3 f4` for
dispatch-chain links, `is_*` for predicates, `*_of(id)` for accessors,
`parse_X`/`scan_X`/`fill_X` for "collect a delimited list".

## Global state

An `export` is a *statement*, so it runs when its function runs. Every project
has a `setup()` called first, split into `init_*` functions of 3 exports each:

```
setup() {
  init_tokens();
  init_nodes();
  init_syms();
} return void;

init_tokens() {
  export string[] tkind = [];
  export string[] ttext = [];
  export int[] tline = [];
} return void;
```

Read with `(import tkind)`. Append with `push((import tkind), kind)`.

**Writing an element of an imported list needs a helper.** `(import xs)[0] = v`
compiles to a *comparison* and silently does nothing — the parser only
recognises index-assignment when the statement starts with a plain identifier.
Passing the list as a parameter makes the target an identifier again, and
lists are reference-semantic, so the store is shared:

```
lset(int[] xs, int i, int v) {
  xs[i] = v;
} return void;

bump() {
  lset((import counter), 0, (import counter)[0] + 1);
} return void;
```

**Only the declaring function may assign an exported scalar.** Anything
several functions must mutate lives in a one-element list.

## Records as parallel lists

There are no structs. A record is one row across several lists, addressed by
an integer id:

```
newnode(string k, int a, int b, string s) {
  push((import nk), k);
  push((import na), a);
  push_rest(b, s);
} return int len((import nk)) - 1;

k_of(int id) {
} return string (import nk)[id];

a_of(int id) {
} return int (import na)[id];
```

A map is two parallel lists plus a linear search:

```
find_str(string[] names, string name) {
  int i = 0;
  int found = 0 - 1;
  while(i < len(names)) {
    found = match_idx(names, i, name, found);
    i = i + 1;
  }
} return int found;

match_idx(string[] names, int i, string name, int found) {
  int r = found;
  if(names[i] == name) {
    r = i;
  }
} return int r;
```

## Building text

`"" + n` converts a number. `+` concatenates.

```
join(int[] xs) {
  string out = "";
  int i = 0;
  while(i < len(xs)) {
    out = out + sep(i) + xs[i];
    i = i + 1;
  }
} return string out;

sep(int i) {
  string s = "";
  if(i > 0) {
    s = ", ";
  }
} return string s;
```

There is no substring builtin; write one:

```
slice(string src, int a, int b) {
  string out = "";
  while(a < b) {
    out = out + chr(charat(src, a));
    a = a + 1;
  }
} return string out;
```

Indentation is threaded as a parameter, deepened with `ind + "    "`.

## A cursor over a token stream

```
cur_kind(int[] pos) {
  string kind = "eof";
  if(pos[0] < len((import tkind))) {
    kind = (import tkind)[pos[0]];
  }
} return string kind;

advance(int[] pos) {
  pos[0] = pos[0] + 1;
} return void;
```

Created by the driver as `int[] pos = [0];`.

## Systems primitives (added for this port)

```
word p = alloc(64);            // zeroed, bounds-checked, never 0
poke32(p + 8, 0x04030201);
word v = peek32(p + 8);
word b = peek8(p + 8);         // 1 -- the store is little-endian
```

`word` is 64-bit. `& | ^ ~ << >>` work; `>>` is arithmetic. Unsigned versions
are `udiv`, `umod`, `ult`, `ushr`. Hex literals are supported. Bitwise binds
**tighter** than comparison, so `flags & MASK != 0` needs no parentheses.

## Traps

1. `(import xs)[i] = v` silently does nothing — use `lset`.
2. Two constant functions returning the same literal collide. Give each magic
   number **one** name and call it.
3. "Just like X but for Y" must become a parameter, not a copied function.
4. A bare `[]` needs a known list type: `int[] none = [];` first.
5. Never mutate and read the same structure in one expression — the order is
   unspecified.
6. `return` inside a body is an error; it goes after `}`.
7. A name may not collide with a function name, or with an exported global.
