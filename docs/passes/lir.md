# LIR passes

These passes operate on `qoalahost` modules. Source: `include/Dialect/QoalaHost/Passes.td`.

## `qoalahost-add-block-precedences`

Materializes inter-block predecessor edges in `qoalahost.blk_meta`. The runtime uses these edges to enforce ordering between blocks that the static scheduler would otherwise be free to reorder. With `use-online-scheduler=true`, the pass additionally adds conservative floater-to-branch dependencies for EPR (QC) blocks required by the online scheduler; with the default `false`, EPR blocks are not forced before branch blocks, so the static scheduler can pipeline EPR generation with Pauli corrections. This is a mandatory pass before `qoala-translate --mlir-to-iqoala` — the runtime expects every block's `blk_meta` to have its `predecessors` populated.

- **Pass class:** `QoalaHostAddBlockPrecedences`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:**
    - `use-online-scheduler: bool` (default `false`).
- **Dependent dialects:** `qoalahost`, `func`.

## `qoalahost-reorder-blocks`

Reorders the blocks in `qoalahost.main_func` to minimize total qubit lifetime. The pass is implemented as a MILP that takes the current block schedule, the cost-model flags exposed by `qoala-opt` (`--qoala-opt-link-duration`, `--qoala-opt-single-gate-duration`, `--qoala-opt-two-gate-duration`, `--qoala-opt-host-instr-time`, `--qoala-opt-qnos-instr-time`, `--qoala-opt-qubit-lifetime`, and the rest), and the inter-block edges materialized by `qoalahost-add-block-precedences`, and produces a permutation of the blocks that respects the precedences while shortening lifetimes. The underlying MILP is solved with SCIP. The pass is optional: skip it if you only care about correctness, since the unreordered LIR is still valid. With `with-deadlines=true` it additionally computes and stores per-block deadlines into `blk_meta.deadlines` — enable this when the runtime is expected to use them.

- **Pass class:** `QoalaHostReorderBlocks`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:**
    - `with-deadlines: bool` (default `false`).
- **Dependent dialects:** `qoalahost`, `func`.

## Analysis-print passes

The four `qoalahost-show-analysis-*` passes do not transform the IR; they run their analysis and print the result to the analysis sink. They are observers, safe to insert anywhere in a pipeline.

### `qoalahost-show-analysis-qmem-eff`

Prints the quantum-memory-efficiency analysis computed in LIR.

- **Pass class:** `QoalaHostQMemEffShowAnalysis`.
- **Anchor:** `qoalahost::MainFuncOp`.
- **Options:** none.
- **Dependent dialects:** `qoalahost`, `func`.

### `qoalahost-show-analysis-qubit-life`

Prints the qubit-lifetime analysis.

- **Pass class:** `QoalaHostQubitLifeShowAnalysis`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:** none.
- **Dependent dialects:** `qoalahost`, `func`.

### `qoalahost-show-analysis-gate-count`

Prints the gate-count analysis. The pass consults the `--qoala-opt-single-gate-duration` and `--qoala-opt-two-gate-duration` cost-model flags.

- **Pass class:** `QoalaHostGateCountShowAnalysis`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:** none.
- **Dependent dialects:** `qoalahost`.

### `qoalahost-show-analysis-esp`

Prints the estimated success probability (ESP) analysis. The pass consults the gate/two-gate error rates and the link/qubit-lifetime cost-model flags.

- **Pass class:** `QoalaHostESPShowAnalysis`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:** none.
- **Dependent dialects:** `qoalahost`, `func`.
