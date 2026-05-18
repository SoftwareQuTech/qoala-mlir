# Architecture

The `qoala-mlir` toolchain is structured around three intermediate representations and two tools that allow hybrid quantum programs to be compiled and optimized through them.

![Backend overview](../assets/figures/backend-overview.svg)

This section drills into the two structural pieces of the toolchain. [The three IRs](irs.md) covers what each of the `qnet`, `qmem`, `qoalahost`, `netqasm`, and `qremote` dialects expresses, and how the shape of a program changes between HIR, MIR, and LIR. [Pass pipeline](pipeline.md) covers the order in which passes are typically composed, distinguishing the ones that are mandatory for correctness from those that are pure optimizations. Per-op signatures live in the [Operations reference](../reference/index.md), and per-pass options and behavior live in the [Passes reference](../passes/index.md).

## Module layout

The `include/` and `lib/` directories mirror each other, with `include/Dialect/<DialectName>/` holding the TableGen definitions and headers and `lib/Dialect/<DialectName>/` holding the implementation. The same mirror applies to `Conversion/` (one subdirectory per IR-to-IR conversion), `Analysis/` (which contains the dialect-specific analyses and the MILP-based reordering), and `Target/iQoala/` (the LIR-to-`.iqoala` translation). The Python bindings live under `lib/Python/` and produce the `qnet` package; a small C bindings shim sits in `lib/QNet-CAPI/`.

```
qoala-mlir/
├── include/
│   ├── Dialect/{QNet,QMem,QoalaHost,NetQASM,QRemote}/   # *.td definitions + headers
│   ├── Conversion/{QoalaHIRToQoalaMIR,QoalaMIRToQoalaLIR}/
│   ├── Analysis/{QNet,QMem,QoalaHost,NetQASM,Helpers}/
│   ├── Target/iQoala/                                   # LIR → .iqoala translation
│   └── Tools/QoalaOpt.h                                 # extern decls for qoala-opt CLI flags
├── lib/                # implementation, mirroring include/
│   ├── Python/         # MLIR Python bindings (package: qnet)
│   └── QNet-CAPI/      # C bindings
├── tools/{qoala-opt,qoala-translate}/
└── llvm/               # LLVM submodule
```

## Two compilation tools

`qoala-opt` is the main tool of the compiler — it provides the optimization pipeline for hybrid quantum programs. The tool registers all five qoala dialects, all built-in MLIR passes, every dialect-specific pass declared under `Dialect/.../Passes.td` (and its conversion equivalents), and twelve cost-model `cl::opt` flags. The full surface is documented in [tools/qoala-opt](../tools/qoala-opt.md).

`qoala-translate` is the backend tool: it provides the final translation step from a fully lowered LIR module into the `.iqoala` format consumed by the Qoala runtime. It registers a single translation, `--mlir-to-iqoala`, and no cost-model flags of its own. The full surface is documented in [tools/qoala-translate](../tools/qoala-translate.md).
