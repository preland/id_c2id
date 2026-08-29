# Front-end integration harness

Drives the real lexer, expression parser and statement parser over C on stdin
and prints the parse tree, so that "the front end works" is a command anyone
can run rather than a claim.

```sh
tests/frontend/build.sh          # builds tests/frontend/parse
echo 'if (x) { g(); }' | build/frontend/parse
```

`harn/stub/ty/` stands in for `c2id/parse/ty/` until it lands. It recognises
only `int`, `char` and `long` as type names, which is enough for statements
and expressions but means a cast to any other type is reported as
`expression expected` — that is the stub, not the parser:

```
p = (char *)q + 1;      parses
p = (void *)q;          expression expected, found ')'
```

Delete the stub and the `harn/stub` directory shrinks to the printer once the
real type module is in.
