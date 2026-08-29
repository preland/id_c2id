# `ex/` — the C expression parser

Recursive descent over the token cursor, producing the expression nodes of
`NOTES.md` §4. 101 functions in 36 files.

Owned by other modules and **not** defined here: `cur_kind`, `cur_text`,
`nxt_text`, `advance`, `at_text` (the cursor, `c2id/lex/drv/`);
`parse_typename`, `is_type_start` (`c2id/parse/ty/`); `newnode`, `newleaf`,
`newlist`, `k_of`/`a_of`/`b_of`/`s_of`/`t_of`/`l_of`/`m_of` (`c2id/parse/ast/`).

## Entry points

| function | what it parses | used by |
| --- | --- | --- |
| `parse_expr(pos)` | a full comma expression | expression statements, `for` clauses, `(...)`, `[...]` |
| `parse_assign(pos)` | one assignment-expression, no comma | argument lists, initialisers |
| `parse_cond(pos)` | a conditional expression = C's *constant-expression* | array bounds, `case` labels, enum values |
| `expr_str(id)` | — | fully parenthesised printer, for tests and emitter diagnostics |

## Precedence, loosest first

Everything from `||` down to `%` is one table-driven ladder: `parse_bin(pos,
lvl)` folds level `lvl` and parses its operands at `lvl + 1`. C has ten such
levels that differ only in which operator strings they name, which is exactly
the "just like X but for Y" shape `id` forbids copying, so the level is a
parameter and the operator set is a table (`p/lad/bin/tab/tab.id`).

| # | level | operators | how | assoc |
| --- | --- | --- | --- | --- |
| 1 | comma | `,` | `parse_expr` | left |
| 2 | assignment | `=` `+=` `-=` `*=` `/=` `%=` `<<=` `>>=` `&=` `^=` `\|=` | `parse_assign` | **right** |
| 3 | conditional | `? :` | `parse_cond` | **right** |
| 4 | logical or | `\|\|` | `parse_bin` lvl 0 | left |
| 5 | logical and | `&&` | `parse_bin` lvl 1 | left |
| 6 | bitwise or | `\|` | `parse_bin` lvl 2 | left |
| 7 | bitwise xor | `^` | `parse_bin` lvl 3 | left |
| 8 | bitwise and | `&` | `parse_bin` lvl 4 | left |
| 9 | equality | `==` `!=` | `parse_bin` lvl 5 | left |
| 10 | relational | `<` `>` `<=` `>=` | `parse_bin` lvl 6 | left |
| 11 | shift | `<<` `>>` | `parse_bin` lvl 7 | left |
| 12 | additive | `+` `-` | `parse_bin` lvl 8 | left |
| 13 | multiplicative | `*` `/` `%` | `parse_bin` lvl 9 | left |
| 14 | cast | `(T) e` | `parse_cast` | right |
| 15 | unary | `!` `~` `+` `-` `*` `&` `++` `--` `sizeof` | `parse_unary` | right |
| 16 | postfix | `f(a)` `a[i]` `s.f` `p->f` `x++` `x--` `(T){...}` | `parse_postfix` | left |
| 17 | primary | identifier, constant, string, `(e)` | `parse_primary` | — |

Two consequences worth stating, because both are C oddities the table encodes:
shift (11) is **looser** than additive (12), so `1 << 2 + 3` is `1 << (2 + 3)`;
and the bitwise levels sit between `&&` and `==`, so `a | b & c ^ d` is
`a | ((b & c) ^ d)`.

## File map

```
ex/
  p/                            the parser
    lad/                        comma, assignment, conditional, binary ladder
      top.id      parse_expr  fold_comma  parse_assign
      asg.id      assign_tail  is_assignop  parse_cond
      bin/
        bin.id    cond_tail  parse_bin  parse_next
        fold.id   fold_bin  op_level  level_at
        tab/
          tab.id  op_names  op_levels  find_op
          find.id match_op  cond_else
    un/                         cast, unary, sizeof
      cast/
        cast.id   parse_cast  paren_head  paren_body
        cast2.id  cast_body  cast_tail  cast_rest
        lit.id    paren_expr  compound_lit  scan_inits
      op/
        op.id     parse_unary  is_unop  unary_op
        op2.id    parse_unary2  is_incdec  pre_incdec
        op3.id    parse_unary3
      sz/
        sz.id     parse_sizeof  sizeof_arg  sizeof_paren
        sz2.id    sizeof_body  sizeof_type  sizeof_inner
        sz3.id    sizeof_expr
    pf/                         postfix, primary, literal decoding
      post/
        p1.id     parse_postfix  postfix_tail  is_postfix_start
        p2.id     postfix_one  postfix2  postfix3
        call/
          call.id   parse_call  scan_args  fill_list
          call2.id  skip_comma  parse_index  post_incdec
          mem.id    parse_member  member_node
      prim/
        q1.id     parse_primary  primary2  primary3
        q2.id     primary4  parse_var  parse_const
        str/
          q3.id     const_val  prim_paren  prim_bad
          str.id    parse_str  str_more  str_join
      lit/
        dig.id    dig_val  alpha_dig  upper_dig
        num/
          n1.id     to_num  is_float_lit  is_hex_lit
          n2.id     has_float_ch  float_ch_at  int_val
          n3.id     dec_or_oct  is_oct_lit  digits_val
        chr/
          c1.id     chr_val  first_ch  esc_val
          c2.id     esc_val2  esc_named  esc_at
          c3.id     esc_chars  esc_vals
  s/                            expr_str, the fully parenthesised printer
    s1.id       expr_str  es2  es3
    s2.id       es4  es5  es6
    sub/
      s3.id     es7  num_str  bin_str
      s4.id     cond_str  sizeof_str  type_ref
      s5.id     call_str  args_str  arg_sep
```

## How the awkward parts are done

**`(` is three-ways ambiguous** — cast, parenthesised expression, or compound
literal. There is no two-token lookahead helper and `is_type_start` reads the
*current* token, so `parse_cast` consumes the `(` first (`paren_head`) and
decides after (`paren_body`). Nothing is lost: every branch wanted it consumed.
`sizeof (` does the same, in `sz/`.

**A parenthesised expression re-enters postfix.** `paren_expr` ends with
`postfix_tail`, so `(*p)[i]`, `(f)(x)` and `(a).b` chain correctly.

**Argument lists use `parse_assign`, not `parse_expr`**, which is the whole
reason `parse_assign` is public: a comma between arguments is a separator, not
the comma operator. `f(a, b)` gives a `call` with two arguments; `(a, b)` gives
one `bin` node with operator `,`.

**Adjacent string literals concatenate** (`"a" "b"` is one `str` node) —
preprocessed C produces them constantly. This assumes the lexer puts the text
*between* the quotes into `ttext`, with escapes unresolved, so joining is plain
concatenation and the emitter re-quotes once.

**Number and character literals are decoded here.** `to_num` handles decimal,
hex `0x…`, octal `0…`, and any tail of `u/U/l/L` suffixes (the digit scan stops
at the first character that is not a digit in the chosen radix, which drops the
suffix for free). `chr_val` handles `\a \b \t \n \v \f \r \" \' \? \\`, octal
`\0`…`\377`, and hex `\xNN`; an unrecognised escape stands for the character
itself. `to_int` is not used anywhere — it only knows decimal and would
silently mistranslate `0x20` and `010`.

**Node ids as fields.** `bin` and `assign` have the same three fields in the
same places, so they share one printer. `un` vs `post` is how `++*p` is told
from `*p++`.

## Deliberate deviations, and what is not done

* **Floating-point literals are not evaluated.** `na` is 0 and the spelling
  lives in `ns`; `is_float_lit` is what recognises them (a `.`, `e` or `E` in a
  non-hex literal). Hex floats (`0x1p3`) are not recognised as floats. Whoever
  adds float support should read `ns`, not `na`. `expr_str` prints the spelling
  for a float and the decoded value for an integer, which is how the decoder is
  tested.
* **Compound literals `(T){…}` extend the ABI.** `NOTES.md` §4 has no node kind
  for a brace initialiser, so `(T){a, b}` becomes a `cast` whose operand is a
  `call` node with **callee -1** and the elements as its argument list. -1 is
  never a node id and the ABI already spells "absent" that way (`decl`, `case`,
  `return`). Nested braces are not flattened — an inner `{` is handed to
  `parse_assign`, which does not know about it, so `{{1,2},{3,4}}` will not
  parse. If an `init` kind is ever added, `p/un/cast/lit.id` and
  `s/sub/s5.id` are the only two places to change.
* **`sizeof` distinguishes its two forms by `nb`.** `nb != 0` means
  `sizeof(T)`. The ABI says `na` is "operand or 0", but 0 is a legitimate node
  id, so both fields are ambiguous in principle; in practice node 0 is created
  long before any expression is parsed.
* **`_Generic`, `_Alignof`, `__builtin_*`, and GCC statement expressions
  (`({...})`) are not handled.** Neither are digraphs.
* **Multi-character constants** (`'ab'`) take the value of the first character.
* **Wide/UTF literals** (`L'a'`, `u8"x"`) are not recognised; the prefix is
  assumed to have been dealt with by the lexer.
* **No constant folding and no type checking.** `parse_cond` returns a tree; a
  later pass has to evaluate it for an array bound or a case label.
* **Error recovery is minimal.** A token that cannot start an expression is
  reported by `prim_bad`, consumed so no loop spins, and replaced by a `num` 0.
* **`expr_str` prints a type as `t<node id>`** rather than reconstructing the C
  declarator: types belong to `parse/ty/`, and this module deliberately does not
  reach into them.

## Naming

`NOTES.md` §3 reserves `t` for a type node id, but `parse/ast/mk/new.id`
already declares `string t` in `newnode`, and a name carries one type
program-wide. **A type node id is therefore called `ty` (int) throughout
`parse/code/`.** Other names introduced here, for the registry: `op` (string,
an operator token), `node` / `left` / `right` / `lvl` / `radix` (int), `ops`
(string[], a table of tokens), `lvls` (int[], the values beside it).
