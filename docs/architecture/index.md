# Architecture

The `qoala-mlir` toolchain is structured around three intermediate representations and two entry-point binaries that drive them.

![Backend overview](../assets/figures/backend-overview.svg)

This section covers:

- **[The three IRs](irs.md)** — the `qnet`, `qmem`, `qoalahost`, `netqasm`, `qremote` dialects and how a program's shape changes between HIR, MIR, and LIR.
- **[Pass pipeline](pipeline.md)** — the order in which passes are typically composed, which passes are mandatory for correctness, and which are optimizations.

For per-op signatures, see the [Operations reference](../reference/index.md). For per-pass options and behavior, see the [Passes reference](../passes/index.md).

## Module layout

The `include/` and `lib/` directories mirror each other:

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
├── tools/{qoala-opt,qoala-translate,interpreter}/
└── llvm/               # LLVM submodule
```

## Two entry-point binaries

- **`qoala-opt`** is the main tool of the compiler, aiming to provide the optimization pipeline for quantum hybrid programs. This tool registers all five qoala dialects, all built-in MLIR passes, every dialect-specific pass declared under `Dialect/.../Passes.td` (and its conversion equivalents), and twelve cost-model `cl::opt` flags. See [tools/qoala-opt](../tools/qoala-opt.md).
- **`qoala-translate`** acts as the "backend" tool that provides a final translation step from an already-optimized quantum hybrid program into a fomat that can be executed by a quantum simulator. This tool registers a single translation, `--mlir-to-iqoala`, which prints the textual `.iqoala` form of an LIR module. It does not register any cost-model flags of its own. See [tools/qoala-translate](../tools/qoala-translate.md).
