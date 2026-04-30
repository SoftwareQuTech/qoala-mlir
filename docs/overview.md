# Overview

`qoala-mlir` lowers a quantum-network program through three intermediate representations and then translates the result to the executable `.iqoala` format consumed by the Qoala runtime.

![Big picture: full pipeline](assets/figures/pipeline-overview.svg)

## The pipeline

A program reaches `qoala-mlir` as a `qnet` (Qoala HIR) module emitted by the [euqalyptus](<EUQALYPTUS_DOCS_URL>) frontend. From there:

1. **Optimizations on HIR** — the `qnet` dialect expresses quantum operations as SSA value-to-value transformations. Passes like `qnet-peephole-optimizations` and `qnet-dead-code-elimination` rewrite the program at this level, where reasoning about gate cancellation and rotation merging is convenient.
2. **HIR → MIR** (`lower-qoala-hir-to-mir`) — quantum SSA values are lowered to explicit memory locations (`i32` qubit pointers) in the `qmem` dialect. Quantum ops become side-effecting calls on those pointers.
3. **MIR helpers** — passes like `lower-float-rotations`, `unfold-comm-ops`, `functionize`, and `fold-constants` normalize the MIR program in preparation for lowering.
4. **MIR → LIR** (`lower-qoala-mir-to-lir`) — the single mixed classical/quantum function in `qmem` is split: classical control becomes `qoalahost` blocks, quantum work becomes standalone `netqasm` routines, and remote-node names live in `qremote`.
5. **LIR finalization** — `qoalahost-add-block-precedences` records inter-block ordering, `qoalahost-reorder-blocks` (optionally MILP-driven) shortens qubit lifetimes, and `qoala-translate --mlir-to-iqoala` emits the textual `.iqoala` executable.

![Backend pipeline detail](assets/figures/backend-overview.svg)

## The three IRs at a glance

| IR | Dialect(s) | What it expresses |
| --- | --- | --- |
| **HIR** | `qnet` | Value-based quantum semantics. Qubits are SSA values of type `!qnet.qubit`. Passes here are concerned with algebraic equivalences over gates. |
| **MIR** | `qmem` | Explicit quantum-memory model. Qubits are `i32` pointers; ops are side-effecting. Convenient for layout, lifetime, and angle-discretization concerns. |
| **LIR** | `qoalahost` + `netqasm` + `qremote` | Runtime-shaped form: a classical host body (`qoalahost.main_func`) calls quantum routines (`netqasm.local_routine`, `netqasm.request_routine`); remote nodes are referenced by symbol (`qremote.remote`). |

For the deeper version, see [The three IRs](architecture/irs.md) and the per-dialect reference under [Operations reference](reference/index.md).

## Tooling at a glance

- **[`qoala-opt`](tools/qoala-opt.md)** — registers all standard MLIR passes plus the dialect-specific passes documented in the [Passes reference](passes/index.md). It also exposes a small set of cost-model flags (`--qoala-opt-single-gate-duration`, `--qoala-opt-link-duration`, …) used by analyses and the MILP-based block reorderer.
- **[`qoala-translate`](tools/qoala-translate.md)** — registers a single translation, `--mlir-to-iqoala`, which prints the `.iqoala` form of an LIR module.
- **[Python bindings (`qnet`)](bindings.md)** — the `qoala-mlir` build also produces an MLIR Python bindings package. This is what [euqalyptus](<EUQALYPTUS_DOCS_URL>) imports under `qnet.dialects.qnet` to construct HIR programmatically.

## Cross-references

- The frontend that produces the input HIR: [euqalyptus](<EUQALYPTUS_DOCS_URL>).
- An invariant or behavior question: prefer the [Operations reference](reference/index.md) and the TableGen `.td` files under `include/Dialect/` over the design paper, which may have drifted.
