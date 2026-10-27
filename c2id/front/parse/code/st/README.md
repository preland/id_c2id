# Statements

`parse_stmt` is the entry point. C has fifteen statement forms and `id` allows
three actions per block, so the choice is made one question at a time down the
chain `parse_stmt → parse_stmt2 → … → parse_stmt15`, each link asking about
exactly one keyword. That is the standard `id` shape for what would elsewhere
be a `switch`, and it is why these functions look like copies of one another —
each names a different string literal and a different callee, which is exactly
what `id`'s duplicate-logic rule counts as a genuine difference.

The bodies live in `../sb/`. Node kinds and field meanings are fixed by
`../../../NOTES.md` §4.

Two things deliberately *not* done here:

* **`switch` keeps its body as one statement**, with the `case` labels sitting
  inside it as ordinary statements. Turning those into jump edges is the CFG
  builder's job — which is why `switch` needs no special shape in the parser.
* **A `goto`'s target stays a name.** It can only become a block number once
  the whole function has been read.

A multi-declarator declaration (`int a = 1, *b, c[3];`) becomes a `block` of
`decl` nodes, so nothing downstream needs a multi-declarator case.

## Integration contract

This module calls, but does not own:

| from | function |
| --- | --- |
| `lex/drv/` | `cur_kind`, `cur_text`, `nxt_text`, `advance`, `at_text` |
| `parse/code/ex/` | `parse_expr`, `parse_assign`, `parse_cond` |
| `parse/ty/` | `is_type_start`, `parse_declspec`, `parse_declarator`, `decl_name` |
| `parse/ast/` | `newnode`, `newleaf`, `newlist` |

It owns `skip_semi` and `skip_comma`.

## Verified

Parsed from hand-fed token streams, printed as trees:

```
if (c) { x; } else { y; }   → (if a0 (block (expr)) (block (expr)))
while (c) { continue; }     → (while a0 (block (continue)))
for (i; c; s) { b; }        → (for cond=2 step=3 (expr) (block (expr)))
do { b; } while (c);        → (dowhile a3 (block (expr)))
switch (v) { case k: b; default: break; }
                            → (switch (block (case a1) (expr) (case a-1) (break)))
{ top: goto top; }          → (block (label top) (goto top))
int n = z;                  → (block (decl n init=1))
```

`default` is recorded as a `case` whose value is -1 — no C constant expression
can collide with that, and it keeps the CFG builder to one node kind.
