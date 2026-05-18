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

All durations are in ticks; the deadline-estimation and reordering MILPs treat one tick as one nanosecond.

| Flag | Type | Default | Description |
| --- | --- | ---: | --- |
| `--qoala-opt-single-gate-duration` | `uint32` | 10 | Duration of a single-qubit gate. |
| `--qoala-opt-single-gate-error` | `float` | 0.01 | Error probability of a single-qubit gate (used by the ESP analysis). |
| `--qoala-opt-two-gate-duration` | `uint32` | 50 | Duration of a two-qubit gate. |
| `--qoala-opt-dual-gate-error` | `float` | 0.05 | Error probability of a two-qubit gate (used by the ESP analysis). |
| `--qoala-opt-latency` | `uint32` | 100 | Per-receive base latency for classical communication operations. The duration of a `qoalahost.recv_int`/`recv_float` is `qoalaOptLatency + qoalaOptHostPeerLatency` (for tensor variants `recv_ints`/`recv_floats`, multiplied by the number of values). Note: no `desc()` string is registered for this flag in the source, so it appears without a description in `qoala-opt --help`. |
| `--qoala-opt-link-duration` | `uint64` | 1000 | Time taken by a single entanglement-generation attempt (a `qnet.eprs` / `qmem.eprs` / `netqasm.eprs` link). |
| `--qoala-opt-host-instr-time` | `uint32` | 1 | Time taken by a single classical instruction executed on the **host** (CPS) side. Combined with operation arity in `getDuration()` to model durations of `qoalahost.*` ops. (The string registered as `desc()` in the source erroneously refers to the QNPU.) |
| `--qoala-opt-host-peer-latency` | `uint32` | 1 | Per-message network latency for classical communication. Together with `--qoala-opt-latency`, sets the duration of receive operations (see the row above for the formula). |
| `--qoala-opt-qnos-instr-time` | `uint32` | 1 | Time taken by a single classical instruction executed on the **QNPU/QPS** side (the embedded classical processor inside the quantum stack). Used by NetQASM classical sub-instructions. |
| `--qoala-opt-qubit-lifetime` | `uint64` | 500 | Maximum allowed lifetime of a qubit (`L_max` in the deadline MILP). The same value is reused as the dephasing time `T₂` by the fidelity-estimate analysis. |
| `--qoala-opt-group-ent-reqs` | `bool` | `false` | If true, the functionization pass groups entanglement requests targeting the same remote into a single request routine. Reflects the current near-term-hardware constraint of one outstanding request routine per remote per program. |
| `--qoala-opt-program-horizon` | `uint32` | 0 | Upper bound on total execution time used by the deadline MILP. The default `0` is a sentinel meaning "derive automatically from the program"; the deadline MILP then uses `H = 2 × Σ duration(op)`. A user-supplied positive value below `Σ duration(op)` is rejected with a warning and the default is used instead. |

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
