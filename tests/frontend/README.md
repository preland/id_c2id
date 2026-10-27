# Front-end integration harness

Drives the real lexer, expression parser and statement parser over C on stdin
and prints the parse tree, so that "the front end works" is a command anyone
can run rather than a claim.

```sh
tests/frontend/build.sh          # builds tests/frontend/parse
echo 'if (x) { g(); }' | build/frontend/parse
```

The harness is c2id itself (`conf.id`, `front/`, `mid/`, `back/`) with the driver
`back/emit/gen/out/top/main.id` left out and `harn/main.id` in its place, because
the parser is no longer a slice that builds alone: it reads the diagnostics, the
type sizes and the constants the folder needs from the other stages. Node
numbers in the printed tree start after the type nodes `boot_a` makes (6), so a
change to the nodes `ty_setup` creates moves them. `harn/stub/` holds the tree
printer and the fixtures its cases use.
