# QRemote (LIR)

The `qremote` dialect is small by design. It defines a single op: `qremote.remote`, used at module scope to declare a remote-node identifier. Other dialects' send/recv/eprs operations refer to these symbols via `FlatSymbolRefAttr` named `remote`.

Source: `include/Dialect/QRemote/{QRemote,QRemoteDialect,QRemoteOps}.td`.

## `qremote.remote`

Module-scope remote-name symbol.

- **Trait:** `Symbol`.
- **Attributes:** `sym_name` (the remote identifier), `sym_visibility?`.
- **Format:** `attr-dict $sym_name`.

```mlir
qremote.remote @Bob
qremote.remote @Charlie
```

The MIR-to-LIR lowering replaces every `qmem.remote` with the equivalent `qremote.remote`. `UsingRemote_Op`-derived ops in `qoalahost`, `netqasm`, and `qmem` resolve their `remote` symbol against the nearest enclosing symbol table — at LIR, that means against the module-level `qremote.remote` declarations.
