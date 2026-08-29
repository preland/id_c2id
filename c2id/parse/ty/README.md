# parse/ty — C types, declarators, typedefs, record layout

This is the part of `c2id` that answers "what type is this, and how big is it".
It reads **preprocessed kernel C** through the shared token cursor and produces
type nodes in the AST store (`../ast/`, NOTES.md §4), plus four registries the
rest of the compiler queries: typedef names, struct tags, struct members, and
enumerator constants.

Nothing here is defined twice, and nothing here defines the cursor
(`cur_kind` / `cur_text` / `nxt_text` / `advance` / `at_text`), the list
setters (`lset` / `sset`), `slice`, or any node constructor or accessor — those
belong to `lex/drv/` and `parse/ast/` and are only called.

One more function is used but not defined here: **`dig_val`**, from
`../code/ex/p/pf/lit/dig.id`. Turning a byte into a digit is the same fold for
an array bound as for an expression literal or a `\xNN` escape, and `id`
rejects a second function with the same logic — so there is one definition and
this module calls it. See `tab/ce/n/num/dig/dig.md`.

Call **`ty_setup()`** once, after `init_nodes*()`, before anything else here.

---

## The grammar accepted

```
decl-specifiers : ( type-specifier | qualifier | attribute )+

type-specifier  : void | char | short | int | long | float | double
                | signed | unsigned | _Bool | __signed__ | __signed
                | struct-or-union-specifier
                | enum-specifier
                | typedef-name              -- only while no type is fixed yet

qualifier       : const | volatile | restrict | static | extern | inline
                | register | auto | typedef | _Atomic | _Noreturn | __thread
                | _Thread_local | __restrict | __restrict__ | __inline
                | __inline__ | __volatile__ | __const | __extension__
                                            -- parsed, discarded

attribute       : ( __attribute__ | __attribute | __asm__ | __asm | asm
                  | __declspec ) '(' balanced ')'
                                            -- parsed, discarded, nesting-safe

struct-or-union-specifier
                : ( struct | union ) attribute* tag? ( '{' member* '}'
                                                       attribute* )?
member          : decl-specifiers ';'                       -- anonymous s/u
                | decl-specifiers m-declarator ( ',' m-declarator )* ';'
m-declarator    : declarator ( ':' const-expr )?            -- bitfield

enum-specifier  : enum attribute* tag? ( '{' enumerator ( ',' enumerator )*
                                         ','? '}' )?
enumerator      : name ( '=' const-expr )?

declarator      : ( '*' ( qualifier | attribute )* )* direct-declarator
direct-declarator
                : name? suffix*
                | '(' declarator ')' suffix*
suffix          : '[' const-expr? ']'
                | '(' params ')'
params          : ( param ( ',' param )* ( ',' '...' )? )?
param           : decl-specifiers declarator

type-name       : decl-specifiers declarator                -- name absent

const-expr      : level1 ( ( '+' | '-' | '<<' | '>>' | '|' | '&' | '^' )
                           level1 )*
level1          : unary ( ( '*' | '/' | '%' ) unary )*
unary           : '-' unary | primary
primary         : integer-literal | enumerator-name
                | '(' const-expr ')' | sizeof '(' type-name ')'
```

Everything above is accepted **in any order and repeated** where C allows it,
so `long unsigned const int volatile x;` parses and yields `unsigned long`.

## The public entry points

| function | meaning |
| --- | --- |
| `parse_declarator(int[] pos, int inner)` | a declarator applied to the specifier type `inner`; returns the full type node |
| `parse_typename(int[] pos)` | specifiers + declarator; for a cast, a `sizeof(T)`, or a whole declaration |
| `parse_specs(int[] pos)` | the specifier list alone |
| `parse_typedef(int[] pos)` | a whole `typedef` declaration, registering every name it declares |
| `parse_su(int[] pos)` / `parse_enum(int[] pos)` | one struct/union or enum specifier |
| `is_type_start(int[] pos)` | 1 if a type begins at the cursor — consults the typedef table |
| `is_typedef(string name)` / `ty_find(string name)` / `ty_add(string name, int ty)` | the typedef and tag registry |
| `type_size(int ty)` / `type_align(int ty)` | `sizeof` / `_Alignof`, in bytes |
| `type_str(int ty)` | a readable rendering, for diagnostics and tests |
| `member_off(int ty, string name)` | byte offset of a member, or -1 |
| `member_type(int ty, string name)` | a member's type node, or -1 |
| `member_count(int ty)` | how many members a record has |
| `has_enum(string name)` / `enum_val(string name)` | enumerator constants |
| `ce_expr(int[] pos)` | an integer constant expression |

### How the declared name comes back

`id` has no second return value and no out-parameter that is not a list, so
the name a declarator declares is written into the **exported one-element
`string[] dname`** and read back with **`decl_name()`**, immediately after the
call:

```
int ty = parse_declarator(pos, base);
string name = decl_name();          // "" for an abstract declarator
```

`parse_declarator` writes `dname` **last**, after any parameter declarators
have run, so a nested parameter name never overwrites the outer one:
after `void (*signal(int, void (*)(int)))(int)`, `decl_name()` is `signal`.
The value is only valid until the next declarator is parsed.

## Registry additions

NOTES.md §3 lists `tynames`/`tykinds` and `synames`/`syoff`/`sysz`. This module
uses `tynames` and `synames`/`syoff`/`sysz` as specified and adds the columns
below; `tykinds` is not used (a type node id is an `int`, so it is kept in an
`int[]`, not stringified).

| name | type | meaning |
| --- | --- | --- |
| `tyids` | `int[]` | parallel to `tynames`: the type node for that name |
| `syids` | `int[]` | parallel to `synames`: that member's type node |
| `sybit` | `int[]` | bit offset of the member within the byte at `syoff` |
| `sywid` | `int[]` | bitfield width in bits, or -1 for an ordinary member |
| `ecnames`, `ecvals` | `string[]`, `int[]` | enumerator name → value |
| `dname` | `string[]` | one cell: the name the last declarator declared |

`tynames` holds **two kinds of key**: a typedef name under its own spelling,
and a struct or union tag under `"struct T"` / `"union T"`. A key with a space
in it cannot collide with an identifier, so one table serves both. An `enum`
tag is not registered — an enum is `int`, and `enum T x;` yields `int`
regardless.

New local names, for the program-wide one-name-one-type rule:

| name | type | meaning |
| --- | --- | --- |
| `ty` | `int` | a type node id. **Not `t`** — `newnode`'s `t` parameter is a `string`, so `t` is a string program-wide |
| `inner` | `int` | the type a declarator or member list is applied to. **Not `base`** — the lexer uses `base` as a `word` (a flat-store address) |
| `sp` | `int[]` | the specifier accumulator, see below |
| `lay` | `int[]` | record layout state `[next free bit, widest member, alignment]` |
| `ps` | `int[]` | parameter type nodes being collected |
| `ev` | `int[]` | one cell: the next enumerator's default value |
| `names` | `string[]` | a table being linearly searched |
| `un` | `int` | 1 if a record is a union |
| `r`, `v`, `n`, `i`, `j` | `int` | a node id / a value / a count / indices |
| `s` | `string` | a token's spelling |

### Node shapes produced

Exactly NOTES.md §4, with one field pinned down:

* `prim` — `na` size, `nb` 1 if signed, `ns` one of the sixteen canonical names.
* `ptr` — `na` pointee. Always 8 bytes.
* `arr` — `na` element, `nb` count, or -1 for `[]`.
* `fn` — `na` return type, `nl` parameter types (array and function parameter
  types decayed to pointers, a lone `void` dropped).
* `rec` — `na` size, `nb` 1 if union, `ns` tag, `nl` member types, and
  **`nm` = `[alignment, row, row, ...]`**: the record's alignment followed by
  one member-table row number per member, in declaration order. It is a list of
  rows rather than a first/count range because laying out a nested anonymous
  struct appends rows of its own part way through the enclosing record's, so
  the enclosing record's rows are not contiguous.

The specifier accumulator `sp` is 11 cells: `sp[0..9]` count how many times
`void char short int long float double signed unsigned _Bool` were seen (the
order of `spec_words()`), and `sp[10]` is the node named by a struct, union,
enum or typedef name, or -1.

### Layout rules implemented

LP64, System V x86-64 — what the kernel assumes.

* `char` 1, `short` 2, `int` 4, `long` 8, `long long` 8, `float` 4, `double` 8,
  `long double` 16, `void` 0, `_Bool` 1, any pointer 8. A scalar's alignment is
  its width (`void`'s is 1).
* A member is placed at the next offset that satisfies its own alignment; a
  struct's size is rounded up to the struct's alignment, which is the widest
  alignment among its members; a union's size is its widest member, likewise
  rounded.
* A bitfield of type `T` is placed at the cursor if it still fits in the
  `sizeof(T)`-wide storage unit it lands in, and otherwise starts a fresh unit.
  A zero-width field never fits, which is exactly what makes it force
  alignment. `syoff`/`sybit` record where it landed, `sywid` how wide it is.
* An anonymous `struct`/`union` member's members become members of the
  enclosing record, with their offsets shifted — so `member_off` never needs a
  recursive search.

Checked against `cc` on this machine: for fourteen records — including
`struct iphdr` and `struct tcphdr` from the kernel's uapi headers, an
anonymous struct member, a zero-width bitfield, a multidimensional array
member and an enum-bounded one — every size, every alignment, every member
byte offset and every bitfield's bit position and width match
`sizeof`/`alignof`/`offsetof` exactly.

## File map

```
tab/                        registries, sizes, printing, constant expressions
  reg/
    tb/setup.id             ty_setup ty_init1 ty_init2
    tb/find.id              ty_init3 ty_init4 find_str
    tb/tyd.id               str_hit ty_add ty_find
    sy/name.id              is_typedef set_dname decl_name
    sy/enum.id              en_add has_enum enum_val
    sy/col/memcol.id        sy_push sy_col1 sy_col2
    sy/col/row.id           sy_row member_count
    mb/find.id              mem_find mem_hit member_off
    mb/num.id               member_type max_int align_up
    mb/size.id              type_size arr_size tsize2
  val/
    num/align.id            type_align talign2 talign3
    num/util.id             align_of_size skip_tok take_tok
    num/recstr.id           ts_rec ts_su
    str/str.id              type_str ts2 ts3
    str/arr.id              ts_arr ts_cnt ts4
    str/fn.id               ts_fn ts_params ts_sep
  ce/
    e/expr.id               ce_expr ce_eloop ce_estep
    e/mul.id                ce_at_e ce_mul ce_mloop
    e/un.id                 ce_mstep ce_at_m ce_un
    p/prim.id               ce_neg ce_prim ce_num
    p/paren.id              ce_prim2 ce_paren ce_name
    p/sizeof.id             ce_sizeof ce_szarg ce_szparen
    n/name.id               ce_name2 ce_ident ce_skip
    n/apply.id              ce_apply ce_ap2 ce_div
    n/num/apply2.id         ce_ap3 ce_mod ce_ap4
    n/num/lit.id            ce_ap5 num_val num_base
    n/num/dig/radix.id      hex_start is_xch radix_val
    n/num/dig/ok.id         dig_ok        (dig.md: dig_val lives in ../code/ex/)

spec/                       declaration specifiers
  kw/base.id                is_base_kw is_qual is_qual2
  kw/qual.id                is_qual3 is_su_kw is_attr
  kw/word/word.id           is_type_word is_tw2 is_tw3
  kw/word/start.id          is_type_start norm_kw
  sk/attr.id                skip_attrs skip_attr skip_paren
  sk/bal.id                 skip_bal bal_step bal_delta
  sk/more/noise.id          close_delta skip_noise skip_attr_ok
  sk/more/ident.id          take_ident
  sp/acc/spec.id            spec_init parse_specs scan_specs
  sp/acc/step.id            spec_step spec_word spec_basic
  sp/acc/tbl.id             bump_spec spec_words has_idx
  sp/tag/tag.id             spec_tag spec_su spec_en
  sp/tag/su.id              spec_td has_base spec_type
  sp/tag/prim/mk.id         mk_prim prim_names prim_sizes
  sp/tag/prim/tbl.id        prim_signs base_codes prim_idx
  sp/tag/prim/idx/sz.id     sign_adj uns_adj sgn_adj
  sp/tag/prim/idx/adj.id    base_idx long_code long_pick
  sp/.../idx/base/pick.id   ll_code int_dflt pick_code
  sp/.../idx/base/td.id     pick_one int_type parse_typedef
  sp/.../idx/base/list.id   td_names td_one decl_more

dcl/                        declarators, records, enums
  d/dcl/dcl.id              parse_declarator apply_stars star_step
  d/dcl/direct.id           star_quals dcl_direct dcl_named
  d/dcl/grp.id              dcl_group grp_suffix grp_inner
  d/sfx/grp.id              is_group grp_inside type_suffix
  d/sfx/sfx.id              ty_suffix2 arr_suffix arr_count
  d/sfx/fn.id               arr_make fn_suffix fn_make
  d/par/param.id            fill_params param_one unstick
  d/par/real.id             param_body param_dots param_real
  d/par/dec/void.id         param_push is_void decay
  d/par/dec/name.id         decay2 parse_typename
  r/su/su.id                parse_su su_kw su_tag
  r/su/tag.id               su_body su_key rec_look
  r/su/new.id               rec_get new_su mk_rec
  r/mem/body.id             su_reg fill_rec rec_members
  r/mem/scan.id             scan_members fill_members member_step
  r/mem/list/list.id        member_decl mem_list mem_one
  r/mem/list/named.id       mem_named mem_place mem_width
  r/mem/list/place.id       mem_anon place place_at
  r/lay/ws.id               place_ws place_bf bf_pos
  r/lay/splice.id           bf_fits un_zero splice
  r/lay/fin/spl2.id         splice_all splice_one splice_mem
  r/lay/fin/end.id          sy_bpos rec_end rec_size
  r/lay/fin/cnt.id          rec_bytes
  e/enum.id                 parse_enum enum_body enum_items
  e/items.id                scan_enums fill_enums enum_one
  e/one.id                  enum_set enum_eq enum_next
```

74 files, 212 functions. The trees are deep because three entries to a
directory and three functions to a file is all there is: a leaf holds nine
functions, and each level above multiplies by three.

## The one genuinely hard part

`int (*g[5])(void)` — `g` is an array of five pointers to functions returning
`int`. The parenthesised group binds to the type the suffixes **after** it
produce, which are not known when the group is reached. So `dcl_group` skips
the group with `skip_bal`, parses the outer suffixes against the base type,
then winds the cursor back and re-reads the group against the finished type.
`pos` is a one-element cell, so winding back is one assignment. The same
mechanism gets `void (*signal(int, void (*)(int)))(int)` right.

Telling a group from a parameter list (`is_group`) is one token of lookahead:
a parameter list starts with a type or is immediately `)`, a group does not —
and "starts with a type" is the question that needs the typedef table.

## Not implemented

* **K&R (pre-prototype) parameter lists.** `int f(a, b) int a, b; { }` — the
  identifier list is mistaken for a parenthesised declarator. Preprocessed
  kernel C has prototypes everywhere; this is not fixable with one token of
  lookahead and was not worth more.
* **`__attribute__((packed))` and `((aligned(N)))` do not change layout.**
  Attributes are skipped correctly but discarded, so a packed struct is laid
  out as if it were not. Kernel code does use `packed`, mostly on wire formats;
  a caller that needs it must read the attribute itself.
* **`_Atomic(T)`**, the parenthesised form. `_Atomic` as a bare qualifier is
  discarded correctly; `_Atomic(int) x;` is not recognised.
* **Octal literals** are read as decimal (`010` is 10, not 8).
* **Constant-expression precedence is two levels, not C's twelve.** `* / %`
  bind tighter than everything else; `+ - << >> | & ^` share one
  left-associative level. `1 << 3 | 1` is right (9); `1 | 1 << 3` is not (8,
  should be 9). Comparisons, `&&`, `||` and `?:` are not evaluated at all —
  they yield 0 by way of the skip rule below.
* **`sizeof expr`** (without parentheses, or with a parenthesised *expression*
  rather than a type) yields 8. Only `sizeof(type-name)` is exact.
* **Character constants** in a constant expression evaluate to 0; the token
  kind `chr` is not decoded.
* **Any other token in a constant expression is skipped and contributes 0.**
  This is deliberate: an array bound the evaluator does not understand yields a
  wrong size rather than hanging the parser. It is also the one place where a
  silent wrong answer is possible, so a caller that cares should check the
  bound it gets back.
* **A struct defined twice** (impossible in one translation unit) would append
  its members twice; there is no guard.
* **Variable-length arrays** are not distinguished — `int a[n]` gets whatever
  `n` evaluates to, which is 0 unless `n` is an enumerator.
