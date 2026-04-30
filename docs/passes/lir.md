# LIR passes

These passes operate on `qoalahost` modules. Source: `include/Dialect/QoalaHost/Passes.td`.

## `qoalahost-add-block-precedences`

Materializes inter-block predecessor edges in `qoalahost.blk_meta`. Used by the runtime to enforce ordering between blocks that the static scheduler would otherwise be free to reorder.

- **Pass class:** `QoalaHostAddBlockPrecedences`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:**
  - `use-online-scheduler: bool` (default `false`) — when `true`, add conservative floater-to-branch dependencies for EPR (QC) blocks required by the online scheduler. With the default (`false`), EPR blocks are not forced before branch blocks, so the static scheduler can pipeline EPR generation with Pauli corrections.
- **Dependent dialects:** `qoalahost`, `func`.

This pass is **mandatory** before `qoala-translate --mlir-to-iqoala`: the runtime expects every block's `blk_meta` to have its `predecessors` populated.

## `qoalahost-reorder-blocks`

Reorders blocks in `qoalahost.main_func` to minimize qubit lifetimes. Implemented as a MILP that takes:

- the current block schedule,
- the cost-model flags (`--qoala-opt-link-duration`, `--qoala-opt-single-gate-duration`, `--qoala-opt-two-gate-duration`, `--qoala-opt-host-instr-time`, `--qoala-opt-qnos-instr-time`, `--qoala-opt-qubit-lifetime`, …),
- the inter-block edges materialized by `qoalahost-add-block-precedences`,

and produces a permutation of the blocks that respects the precedences while shortening lifetimes. SCIP is used as the underlying MILP solver.

- **Pass class:** `QoalaHostReorderBlocks`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:**
  - `with-deadlines: bool` (default `false`) — also compute and store per-block deadlines in `blk_meta.deadlines`. Enable this when the runtime is expected to enforce them.
- **Dependent dialects:** `qoalahost`, `func`.

This pass is **optional**. Skip it if you only care about correctness; the unreordered LIR is still valid.

## Analysis-print passes

These passes do not transform IR. They run their analysis and print the result.

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

Prints the gate-count analysis. Uses the `--qoala-opt-single-gate-duration` and `--qoala-opt-two-gate-duration` cost-model flags.

- **Pass class:** `QoalaHostGateCountShowAnalysis`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:** none.
- **Dependent dialects:** `qoalahost`.

### `qoalahost-show-analysis-esp`

Prints the estimated success probability (ESP) analysis. Uses the gate/two-gate error rates and link/qubit-lifetime cost-model flags.

- **Pass class:** `QoalaHostESPShowAnalysis`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:** none.
- **Dependent dialects:** `qoalahost`, `func`.
