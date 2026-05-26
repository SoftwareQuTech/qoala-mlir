# Passes reference

Every dialect-specific pass declared under `include/Dialect/.../Passes.td` and `include/Conversion/.../*.td` is invokable through `qoala-opt` using its `mnemonic` string, and pass options are passed inside the pass argument: `--pass-name=opt1=val1,opt2=val2`.

The pages in this section are grouped by the IR they target. [HIR passes](hir.md) operate on the `qnet` dialect and include the linearity verifier, the peephole optimizer, the gate-count analysis, and the QDCE pass. [HIR → MIR](hir-to-mir.md) covers the conversion that introduces the explicit qubit-pointer memory model. [MIR helpers](mir.md) collects the utility passes that run on `qmem` modules in preparation for lowering to LIR. [MIR → LIR](mir-to-lir.md) covers the conversion that splits a single mixed-classical/quantum function into a host body, NetQASM routines, and remote symbols. Finally, [LIR passes](lir.md) operate on `qoalahost` modules — block-precedence materialization, MILP-based block reordering, and the analysis-print passes.

Note that the standard MLIR passes (e.g. `--canonicalize`, `--cse`, `--sccp`, `--inline`) are also registered by `qoala-opt` via `mlir::registerAllPasses()` — you can invoke them in the same pipeline.
