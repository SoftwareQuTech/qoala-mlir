# HIR passes

These passes operate on `qnet` modules. Source: `include/Dialect/QNet/Passes.td`.

## `qnet-check-linear`

Verifier. Checks that each `!qnet.qubit` value is used at most once — i.e. that the program respects the linear-types invariant of HIR.

- **Pass class:** `QNetCheckLinear`.
- **Anchor:** any op (no anchor restriction).
- **Options:** none.
- **When to run:** before any rewrite that assumes linearity.

## `qnet-dead-code-elimination`

Quantum dead-code elimination. Drops quantum dataflow whose effect is not observably used.

- **Pass class:** `QNetDeadCodeElimination`.
- **Anchor:** `qnet::FuncOp`.
- **Options:**
  - `with-classical-awareness: bool` (default `false`) — when `true`, only treats measurements whose classical results are observably used as liveness roots. With the default, all measurements act as roots.

## `qnet-peephole-optimizations`

Peephole rewrites: Hermitian self-inverse cancellation, rotation merging, optional Pauli→rotation conversion.

- **Pass class:** `QNetPeepholeOptimizations`.
- **Anchor:** `qnet::FuncOp`.
- **Options:**
  - `hermitian-cancel: bool` (default `true`) — cancel `g g` for Hermitian `g` (`x`, `y`, `z`, `hadamard`, `cnot`, `cz`).
  - `rotation-folding: bool` (default `true`) — merge consecutive same-axis rotations into a single rotation.
  - `pauli-to-rotations: bool` (default `false`) — convert `qnet.x`/`qnet.y`/`qnet.z` to their rotation-form equivalents.
  - `two-pi-epsilon: double` (default `0.0`) — tolerance for recognizing rotation angles equivalent to multiples of 2π (so the rotation can be removed).
  - `normalize-angles: bool` (default `true`) — normalize folded rotation angles to the range [-π, π].

## `qnet-show-analysis-gate-count`

Analysis-print pass. Computes and prints the gate-count analysis on the HIR program.

- **Pass class:** `QNetGateCountShowAnalysis`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:** none.
- **Side effects:** writes to stdout / the analysis sink, does not transform IR.

This pass uses the `qoala-opt` cost-model flags (single/two-gate durations) when scoring the program. See [tools/qoala-opt](../tools/qoala-opt.md).
