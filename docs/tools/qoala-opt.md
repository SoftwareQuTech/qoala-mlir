# qoala-opt

`qoala-opt` is the MLIR-based driver for analyses, optimizations, and lowering. It registers:

- All built-in MLIR dialects (`registerAllDialects`) and passes (`registerAllPasses`).
- The five qoala dialects: `qnet`, `qmem`, `netqasm`, `qoalahost`, `qremote`.
- Every dialect-specific pass declared under `Dialect/.../Passes.td` and the conversion passes under `Conversion/.../*.td` — see the full list in the [Passes reference](../passes/index.md).
- Twelve cost-model `cl::opt` flags (table below).
- The `mlir-print-op-generic`, `--debug`, `--print-ir-*` and `--view-op-graph` flags inherited from `MlirOptMain`.

Source: `tools/qoala-opt/qoala-opt.cpp`.

## Cost-model flags

These flags configure the durations and error rates used by analyses (gate count, qubit lifetime, ESP, qmem-efficiency) and by the MILP-based block reorderer (`qoalahost-reorder-blocks`). They have no effect on lowering correctness.

| Flag | Type | Default | Description |
| --- | --- | ---: | --- |
| `--qoala-opt-single-gate-duration` | `uint32` | 10 | Time taken by a single-qubit gate operation. |
| `--qoala-opt-single-gate-error` | `float` | 0.01 | Error rate of a single-qubit gate operation. |
| `--qoala-opt-two-gate-duration` | `uint32` | 50 | Time taken by a two-qubit gate operation. |
| `--qoala-opt-dual-gate-error` | `float` | 0.05 | Error rate of a two-qubit gate operation. |
| `--qoala-opt-latency` | `uint32` | 100 | Generic latency parameter (no description string in the registration). |
| `--qoala-opt-link-duration` | `uint64` | 1000 | Time taken to generate entanglement (an `eprs` link). |
| `--qoala-opt-host-instr-time` | `uint32` | 1 | Time taken by a classical operation running on the QNPU host. |
| `--qoala-opt-host-peer-latency` | `uint32` | 1 | Latency of receiving a classical communication message. |
| `--qoala-opt-qnos-instr-time` | `uint32` | 1 | Time taken by a classical instruction running on the QNPU. |
| `--qoala-opt-qubit-lifetime` | `uint64` | 500 | Maximum lifetime of a qubit. |
| `--qoala-opt-group-ent-reqs` | `bool` | `false` | Whether to group entanglement requests during analyses. |
| `--qoala-opt-program-horizon` | `uint32` | 0 | Program horizon — the maximum time the program is allowed to take. |

There is also one **hidden** flag:

| Flag | Type | Default | Description |
| --- | --- | ---: | --- |
| `--qoala-opt-unoptimize` | `bool` | `false` | Run the "anti-optimization" passes used to compare worst-vs-best-case program runs. (`ReallyHidden`.) |

## Pass invocation

Pass names come from the `mnemonic` strings in the `Passes.td` files. For example:

```sh
qoala-opt program.mlir \
  --qnet-peephole-optimizations \
  --qnet-dead-code-elimination=with-classical-awareness=true \
  --lower-qoala-hir-to-mir \
  --lower-qoala-mir-to-lir=use-online-scheduler=true,max-ops-per-group=4
```

The wrapper passes (`lower-qoala-mir-to-lir`, `lower-qmem-to-lower-dialects`, etc.) accept their inner pass options *embedded in the pass argument*, **not** as separate top-level flags. `--use-online-scheduler=true` on its own will be rejected.

See [Architecture / Pass pipeline](../architecture/pipeline.md) for the recommended end-to-end ordering.

## Standard MLIR knobs

`qoala-opt` inherits **all** flags from `MlirOptMain` (LLVM's `opt` tool). The most notable ones are mentioned here:

| Flag | Use |
| --- | --- |
| `--debug` | Enable `LLVM_DEBUG` prints across the whole tool. |
| `--debug-only=<filter[,filter]>` | Restrict debug prints to specific `DEBUG_TYPE` filters. |
| `--mlir-print-op-generic` | Print ops in generic form, bypassing custom verifiers (handy in verifier-debug loops). |
| `--mlir-print-assume-verified` | Skip the verifier on print, allowing custom printers without verifier infinite loops. |
| `--mlir-disable-threading` | Serialize pass execution. Useful for deterministic debug output. |
| `--mlir-print-value-users` | Annotate printed IR with value-user comments. |
| `--print-ir-after=<pass>`, `--print-after-all` | Dump IR after a named pass / every pass. |
| `--print-ir-before=<pass>`, `--print-before-all` | Dump IR before a named pass / every pass. |
| `--view-op-graph` | Emit a Graphviz `.gv` representation of the op tree on stderr. |

A typical graph-debug invocation looks like:

```sh
qoala-opt program.mlir --view-op-graph 2>&1 >/dev/null | tee program.gv
xdot program.gv
```

(The exact incantation also appears in the repo's `README.md`.)
