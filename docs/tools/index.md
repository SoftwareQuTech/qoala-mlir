# Tools

The qoala-mlir build produces two binaries under `build/bin/`:

- **[`qoala-opt`](qoala-opt.md)** — runs analyses, optimizations, and lowerings between IRs. This is the workhorse of the toolchain: every pass under [Passes reference](../passes/index.md) is invokable through it.
- **[`qoala-translate`](qoala-translate.md)** — translates a fully lowered LIR module into the textual `.iqoala` executable consumed by the Qoala runtime, via the `--mlir-to-iqoala` translation. It does not register any cost-model flags of its own.
