# HIR passes

These passes operate on `qnet` modules. Source: `include/Dialect/QNet/Passes.td`.

## `qnet-check-linear`

A verifier that checks the linear-types invariant of HIR: every `!qnet.qubit` value must be used at most once. The pass is meant to run before any rewrite that assumes linearity (peephole, dead-code elimination, the HIR-to-MIR lowering, …) — if the invariant is violated, those downstream passes can produce silently wrong IR. The pass has no anchor restriction and no options.

- **Pass class:** `QNetCheckLinear`.
- **Anchor:** any op (no anchor restriction).
- **Options:** none.

## `qnet-dead-code-elimination`

Quantum dead-code elimination drops quantum dataflow whose effect is not observably used by the classical program. The pass walks backward through the SSA def-use chains from a set of liveness roots (by default, all measurements; with `with-classical-awareness=true`, only those measurements whose classical outcomes are observably consumed elsewhere) and removes operations that do not contribute to any live qubit value. The full justification — including why a local rotation on the unmeasured side of a (locally or remotely) entangled pair is safe to remove — is given in the accompanying paper.

- **Pass class:** `QNetDeadCodeElimination`.
- **Anchor:** `qnet::FuncOp`.
- **Options:**
    - `with-classical-awareness: bool` (default `false`) — when `true`, only treats measurements whose classical results are observably used as liveness roots.

## `qnet-peephole-optimizations`

A family of local rewrites that exploit HIR's value-based semantics: Hermitian self-inverse cancellation, rotation merging, optional Pauli-to-rotation conversion, full-turn elimination, and angle normalization. Hermitian cancellation collapses adjacent same-Hermitian pairs (`x x`, `hadamard hadamard`, `cnot cnot`, …) into the identity. Rotation merging folds two consecutive same-axis rotations into a single one with the summed angle; the float and integer rotation families are handled uniformly, including cross-pattern matches between them. The optional Pauli-to-rotation conversion rewrites `qnet.x` / `qnet.y` / `qnet.z` to their `RotX` / `RotY` / `RotZ` equivalents, exposing further folding opportunities. Full-turn rotations (those whose angle is an integer multiple of 2π, optionally up to a configurable tolerance) are eliminated, and the surviving rotation angles are normalized to the range $[-\pi, \pi]$.

- **Pass class:** `QNetPeepholeOptimizations`.
- **Anchor:** `qnet::FuncOp`.
- **Options:**
    - `hermitian-cancel: bool` (default `true`) — cancel `g g` for Hermitian `g` (`x`, `y`, `z`, `hadamard`, `cnot`, `cz`).
    - `rotation-folding: bool` (default `true`) — merge consecutive same-axis rotations into a single rotation.
    - `pauli-to-rotations: bool` (default `false`) — convert `qnet.x` / `qnet.y` / `qnet.z` to their rotation-form equivalents.
    - `two-pi-epsilon: double` (default `0.0`) — tolerance for recognizing rotation angles equivalent to multiples of 2π (so the rotation can be removed).
    - `normalize-angles: bool` (default `true`) — normalize folded rotation angles to the range $[-\pi, \pi]$.

## `qnet-show-analysis-gate-count`

An analysis-print pass that computes the gate-count analysis on the HIR program and writes the result to the analysis sink (stdout in the default `qoala-opt` invocation). The pass does not transform the IR; it consults the `qoala-opt` cost-model flags (single- and two-gate durations — see [tools/qoala-opt](../tools/qoala-opt.md)) when scoring the program.

- **Pass class:** `QNetGateCountShowAnalysis`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:** none.
