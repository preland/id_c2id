# Where `dig_val` lives

`dig_val` (and the `a-f` / `A-F` continuations it delegates to) is defined once
for the whole compiler, in

    c2id/parse/code/ex/p/pf/lit/dig.id

not here. This module needs it for integer literals in array bounds, bitfield
widths and enumerator values; `parse/code/ex/` needs exactly the same fold for
integer literals in expressions and for `\xNN` and `\NNN` character escapes —
and `id` rejects a second function with the same signature and logic, which is
the rule working as intended. A hex digit is a hex digit.

`radix_val` and `dig_ok`, next door, call it.
