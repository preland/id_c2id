# `c2id/lex` — the C lexer

Reads preprocessed C (the output of `gcc -E`) and fills the three parallel
token lists of `../NOTES.md` §3. 70 functions across 14 `.id` files.

## Entry points

| call | does |
| --- | --- |
| `init_tokens()` | declares `tkind`, `ttext`, `tline`. Call from `setup()`, before anything else. |
| `lex_all(string src)` | lexes a whole translation unit, appending to those three lists and finishing with an `eof` token. |
| `cur_kind` `cur_text` `cur_line` `nxt_text` `advance` `at_text` | the cursor the parser reads the stream through. `pos` is `int[] pos = [0];`. |
| `lset` `sset` | the imported-list store helpers. Defined **exactly once** for the whole project, here — `../NOTES.md` §6 pencilled them in for `parse/ast/`, but the lexer needs them first (for `curline` and `curfile`), and a second definition anywhere would be a duplicate-logic error. |

Token kinds: `id` `num` `str` `chr` `op` `eof`. Keywords are **not** separated
out — `int`, `while` and `bsearch` are all `id`, and the parser decides by text.

## Why the source lives in the flat store

Everything below `lex_all` takes a `word base` — the store address of source
byte 0 — instead of a `string src`. That is not stylistic. `charat(s, i)` is
implemented as a bounds check against `strlen(s)`, so it is O(length of the
whole input) per character unless the C compiler can hoist the `strlen` out of
the loop, which it cannot across a dispatch chain of separate functions.
Measured on `build/cpp/lib/bsearch.i` (2.5 MB), a `charat`-based scan of that
shape takes **88 seconds**; the same scan through `peek8` takes **0.010 s**.

So `lex_all` does `mem_of_str(src)` once and every scanner reads `peek8(base + i)`,
which is a constant-time bounds check. Token spellings come from
`str_of_mem(base + a, b - a)`: one `memcpy` per token instead of one allocation
per character, which is also what a `chr()`-concatenation `slice` would cost.

Two consequences to keep in mind when extending this code:

* **Byte 0 is end of input.** `mem_of_str` copies the string's NUL, and
  `lex_mem` allocates an 8-byte zero pad immediately behind it, so the two-byte
  lookaheads several scanners do stay inside the store's bounds check and read
  as 0. No character-class predicate accepts 0, so every scanning loop stops
  there on its own. `ch_eof()` names it.
* **Lookahead is limited to two bytes** past the current index by that pad.

## File map

```
lex/
  ch/                       character classification (15 functions)
    cls.id                  is_space, is_digit, is_alpha
    cls2.id                 is_hspace, is_alnum, is_numcont
    code/
      code.id               ch_dq, ch_sq, ch_nl          -- named byte codes
      code2.id              ch_eof, is_exp, is_expsign
      opset.id              is_op2, is_op3, is_pfx       -- operator spellings
  tok/                      token scanners (32 functions)
    tx/
      txt.id                slice, sub2, sub3            -- store -> string
      runs.id               ident_end, digit_end, eol_pos
      numr.id               skip_h, num_end, num_step
    lt/
      esc.id                adv_esc, quote_end, quote_stop
      quote.id              quote_kind, scan_quoted
      cmt.id                cmt_more, cmt_stop, adv_nl
    sc/
      ops.id                op_len, op_len3, scan_op
      idn.id                scan_ident, ident_pick, emit_ident
      dis/
        cmt2.id             cmt_end, scan_cmt, scan_num
        d1.id               scan_one, scan_hash, scan_word
        d2.id               scan_numop, scan_strop, scan_quop
  drv/                      stdin, token store, cursor (23 functions)
    st/
      store.id              lset, sset, add_tok
      line.id               bump_line, set_line, set_file
      mark.id               at_bol, bol_ok, scan_marker
    mk/
      mark2.id              mark_num, mark_file
      top.id                init_tokens, init_lines, lex_all
      run.id                eol_end, scan_src, lex_mem
    cur/
      cur.id                cur_kind, cur_text, cur_line
      cur2.id               nxt_text, advance, at_text
```

## The dispatch chain

One decision per function, each returning the index just past what it
consumed — the shape the 3-action limit forces (`../NOTES.md` §6).

```
scan_src → scan_one → scan_hash → scan_word → scan_numop → scan_strop → scan_quop
           whitespace  line marker  comment     identifier   number      literal / operator
```

* **whitespace** — `adv_nl` consumes one byte and counts it if it is LF.
* **line marker** — a `#` with nothing but horizontal whitespace before it on
  its line. `scan_marker → mark_num → mark_file` reads the number into
  `curline` and the quoted name into `curfile`, emits **no token**, and skips
  the marker's own newline without counting it — so the number applies to the
  line that follows, which is what `gcc -E` means by it. A `#` line with no
  number (a directive that survived preprocessing) is counted as an ordinary
  line instead. A `#` anywhere else is the `#` operator, and `##` still lexes
  as one token.
* **comment** — `//` to end of line (the LF is left for the whitespace path);
  `/* */` through `adv_nl`, so a comment spanning lines advances `tline`
  exactly as the same lines outside one would.
* **identifier** — `[A-Za-z_][A-Za-z0-9_]*`. `ident_pick` catches the one case
  where that run was really the `L` / `u` / `U` / `u8` prefix of a literal.
* **number** — a C *preprocessing number*: a digit, or `.` then a digit, then
  any run of `[A-Za-z0-9_.]` with `e`/`E`/`p`/`P` followed by a sign counting
  as two characters. That single rule covers `42`, `0x1fUL`, `077`, `1.5f`,
  `1e10`, `3.14e-5` and `0x1p-3` with no special cases, and the token text is
  the untouched source spelling — nothing is converted.
* **literal** — `"…"` and `'…'`, escapes honoured so `\"` and `'\''` do not
  terminate early, text is the raw spelling including quotes and any prefix.
  A newline also stops the scan, so an unterminated literal costs one bad
  token instead of the rest of the file.
* **operator** — longest match: `is_op3`, then `is_op2`, then one byte. There
  is deliberately no `is_op1`; any byte that reaches the end of the chain
  becomes a one-character `op` token, which covers the whole single-character
  set and cannot silently drop a byte.

## Names this directory adds to the `../NOTES.md` §3 registry

| name | type | meaning |
| --- | --- | --- |
| `base` | `word` | store address of source byte 0 |
| `e`, `ne`, `ni` | `int` | end index, new end index, next index |
| `j` | `int` | a second cursor into `src` (only `at_bol` needs one) |
| `v` | `int` | the value `lset` stores |
| `names` | `string[]` | the list `sset` stores into |

Exported globals added: `curline` (`int[]`, one element) and `curfile`
(`string[]`, one element). Both are lists rather than scalars because only the
declaring function may assign an exported scalar.

## Testing

There is no `main` here — it belongs to the driver — so this directory cannot
be built on its own. To test it, copy it next to a `tst/main.id` that calls
`init_tokens()`, `lex_all(read_all())` and prints the lists, and build the
copy with `bin/idc`, which enforces every rule of the language itself.

Verified this way against an independent reference tokenizer: kind, spelling
and line for every token agree exactly on `build/cpp/lib/bsearch.i`
(520 450 tokens, 0.054 s) and `build/cpp/lib/sort.i` (150 376 tokens, 0.016 s),
on a hand-written file covering every token form above, and on empty input,
missing trailing newline, CRLF, an unterminated string and an unterminated
block comment. `curfile` ends at `lib/bsearch.c` / `lib/sort.c` and the last
token's line matches those files' real lengths (36 and 357).
