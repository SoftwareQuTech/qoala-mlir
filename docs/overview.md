# Overview

`qoala-mlir` lowers a quantum-network program through three intermediate representations and then translates the result to the executable `.iqoala` format consumed by the Qoala runtime.

![Big picture: full pipeline](assets/figures/pipeline-overview.svg)

## The pipeline

A program reaches `qoala-mlir` as a `qnet` (Qoala HIR) module emitted by the [euqalyptus](https://softwarequtech.github.io/euqalyptus/) frontend. At the HIR level the program is expressed in SSA value-to-value form, which makes reasoning about gate cancellation, rotation merging, and quantum dead-code elimination convenient; passes such as `qnet-peephole-optimizations` and `qnet-dead-code-elimination` rewrite the program at this level before any lowering happens. The HIR-to-MIR conversion, run via `lower-qoala-hir-to-mir`, then replaces every `!qnet.qubit` SSA value with an explicit `i32` qubit pointer in the `qmem` dialect, turning quantum operations into side-effecting calls on those pointers.

Once the program is in MIR, a small family of helper passes prepares it for the structural changes that turn it into a Qoala-block-shaped LIR. `lower-float-rotations` rewrites floating-point angles to the integer-pair form NetQASM accepts; `unfold-comm-ops` decomposes multi-value classical communication ops into single-value variants; `functionize` extracts contiguous groups of quantum operations into stand-alone NetQASM routines; and `fold-constants` cleans up the constants left behind. The MIR-to-LIR conversion, run via `lower-qoala-mir-to-lir`, then splits the single mixed function into a classical host body expressed in the `qoalahost` dialect, one or more quantum routines in the `netqasm` dialect, and module-scope remote-node declarations in `qremote`.

LIR is the level at which the runtime-facing structure of the program becomes visible and at which scheduling-oriented passes operate. `qoalahost-add-block-precedences` materializes the inter-block ordering edges the runtime relies on, `qoalahost-reorder-blocks` solves a MILP to shorten qubit lifetimes (and, optionally, to compute per-block deadlines), and once the module is finalized, `qoala-translate --mlir-to-iqoala` emits the textual `.iqoala` executable.

![Backend pipeline detail](assets/figures/backend-overview.svg)

## The three IRs at a glance

| IR | Dialect(s) | What it expresses |
| --- | --- | --- |
| **HIR** | `qnet` | Value-based quantum semantics. Qubits are SSA values of type `!qnet.qubit`. Passes here are concerned with algebraic equivalences over gates. |
| **MIR** | `qmem` | Explicit quantum-memory model. Qubits are `i32` pointers; ops are side-effecting. Convenient for layout, lifetime, and angle-discretization concerns. |
| **LIR** | `qoalahost` + `netqasm` + `qremote` | Runtime-shaped form: a classical host body (`qoalahost.main_func`) calls quantum routines (`netqasm.local_routine`, `netqasm.request_routine`); remote nodes are referenced by symbol (`qremote.remote`). |

For the deeper version see [The three IRs](architecture/irs.md) and the per-dialect entries in the [Operations reference](reference/index.md).

## Tooling at a glance

The build produces two command-line tools and a Python bindings package. [`qoala-opt`](tools/qoala-opt.md) is the workhorse: it registers all standard MLIR passes together with the dialect-specific passes documented in the [Passes reference](passes/index.md), and exposes a small set of cost-model flags (`--qoala-opt-single-gate-duration`, `--qoala-opt-link-duration`, and so on) used by the analyses and by the MILP-based block reorderer. [`qoala-translate`](tools/qoala-translate.md) is much narrower in scope — it registers a single translation, `--mlir-to-iqoala`, that prints the `.iqoala` form of an LIR module. Alongside the two binaries, the build produces the [Python bindings (`qnet`)](bindings.md) under `qnet.dialects.qnet`; this is what [euqalyptus](https://softwarequtech.github.io/euqalyptus/) imports to construct HIR programmatically.

## Cross-references

The frontend that produces the input HIR is documented at [euqalyptus](https://softwarequtech.github.io/euqalyptus/). When you need to settle an invariant- or behavior question, prefer the [Operations reference](reference/index.md) and the TableGen `.td` files under `include/Dialect/` over the design paper — the paper describes design intent and may have drifted relative to the implementation.
