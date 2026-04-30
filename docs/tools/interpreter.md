# Interpreter

The `interpreter` binary is a thin wrapper around `mlir::JitRunnerMain` (effectively a copy of upstream `mlir-cpu-runner`). It JIT-compiles MLIR-with-LLVMIR-translation to native code and runs it on the CPU.

It is **not part of the production pipeline**. It does not understand the Qoala dialects beyond what the standard MLIR-to-LLVMIR translations provide, and it does not emit `.iqoala`. It exists for ad-hoc experimentation.

Source: `tools/interpreter/interpreter.cpp`.

## When you might use it

Mostly never. If you need to run a quantum-network program, use the simulator stack — not this binary. If you need to test that classical control flow lowers to LLVM correctly, the upstream `mlir-cpu-runner` is the more standard choice.
