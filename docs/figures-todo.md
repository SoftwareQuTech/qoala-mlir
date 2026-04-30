# Figures to (re)draw

Every entry below corresponds to an SVG stub under `docs/assets/figures/`. Each stub contains the figure title and a short description of the intended content; replace it one-for-one with your final asset (same filename, same path).

## `pipeline-overview.svg`

- **Referenced from:** `docs/index.md`, `docs/overview.md`.
- **Intended content:** Left-to-right flow of the full Qoala pipeline. Boxes for: user Python program → euqalyptus frontend → emitted Qoala HIR (`qnet`) → arrow into `qoala-opt` showing the HIR/MIR/LIR stack as three stacked boxes (HIR=`qnet`, MIR=`qmem`, LIR=`qoalahost`+`netqasm`+`qremote`), with the conversion passes (`lower-qoala-hir-to-mir`, `lower-qoala-mir-to-lir`) labeled on inter-box arrows → arrow into `qoala-translate` (`--mlir-to-iqoala`) → final `.iqoala` file. Highlight the qoala-mlir-owned span (everything from HIR through `.iqoala`).

## `backend-overview.svg`

- **Referenced from:** `docs/architecture/index.md`, `docs/overview.md`.
- **Intended content:** Detailed view of just the qoala-opt + qoala-translate part. Three vertical columns (HIR, MIR, LIR) with the major passes that run while the program is at that level listed inside each. Inter-column arrows labeled with the conversion passes. Final arrow into a `qoala-translate` box producing `.iqoala`.

## `pass-pipeline-default.svg`

- **Referenced from:** `docs/architecture/pipeline.md`.
- **Intended content:** Vertical sequence diagram of the recommended default pipeline: input HIR → `qnet-peephole-optimizations` → `qnet-dead-code-elimination` → `lower-qoala-hir-to-mir` → `lower-float-rotations` → `unfold-comm-ops` → `functionize` → `lower-qmem-to-lower-dialects` → `qoalahost-add-block-precedences` → `qoalahost-reorder-blocks` → output LIR feeding into `qoala-translate --mlir-to-iqoala`. Color-code mandatory vs. optimization passes.

## `dialect-graph.svg`

- **Referenced from:** `docs/reference/index.md`, `docs/architecture/irs.md`.
- **Intended content:** A directed graph of dialect-to-dialect transitions: `qnet` (HIR) → `qmem` (MIR), and `qmem` → {`qoalahost`, `netqasm`, `qremote`} (LIR). Arrows labeled with the conversion pass. Each node lists its headline ops (see SVG stub for the per-dialect lists).

## `qnet-op-anatomy.svg`

- **Referenced from:** `docs/reference/qnet.md`.
- **Intended content:** Anatomy of a representative QNet op (e.g. `qnet.rot_x`): operands on the left (input qubit value, angle), op box in the middle with traits annotated (`QubitOpIface`, `RotationOpIface`), and the result on the right (a *fresh* qubit value). Goal: convey that HIR ops are value-to-value and pure with respect to memory.

## `qmem-memory-model.svg`

- **Referenced from:** `docs/reference/qmem.md`, `docs/architecture/irs.md`.
- **Intended content:** A "quantum memory bank" subdivided into slots, each labeled by an `i32` qubit pointer. Side-effecting ops (`qmem.qalloc`, `qmem.init`, `qmem.rot_x`, `qmem.cnot`, `qmem.measure`) shown as arrows into / out of slots. Contrast with `qnet-op-anatomy.svg`: ops mutate slots rather than producing new SSA quantum values.

## `qoalahost-block-structure.svg`

- **Referenced from:** `docs/reference/qoalahost.md`, `docs/architecture/irs.md`.
- **Intended content:** A `qoalahost.main_func` rectangle with several blocks. Each block shown with its `qoalahost.blk_meta` (block_id, predecessors). Block payloads alternate among (a) a single `qoalahost.call`, (b) a single `recv_*`, (c) a sequence of arith/memref ops terminated by `qoalahost.nop_term`. A separate `netqasm.local_routine` box being called from a block. Optional inset: how `qoalahost-reorder-blocks` swaps two blocks to shorten a qubit lifetime.

## `netqasm-routine.svg`

- **Referenced from:** `docs/reference/netqasm.md`.
- **Intended content:** Two function-shaped boxes side-by-side: a `netqasm.local_routine` and a `netqasm.request_routine`, with a tiny op list inside each (`netqasm.rot_x`, `netqasm.hadamard`, `netqasm.measure` for local; `netqasm.eprs_measure` for request). Show an arrow from a `qoalahost.call` (small inset) into the local routine, and a `qremote.remote @Bob` symbol referenced by the request routine.

## `python-bindings-arch.svg`

- **Referenced from:** `docs/bindings.md`.
- **Intended content:** Build pipeline flowchart for the `qnet` Python package. Inputs: `Dialect/QNet/QNet.td` (canonical TD), `lib/Python/mlir_qnet/dialects/QNetOps.td` (re-include), `lib/Python/QNetExtension.cpp` (manual type registration). CMake step → output package `build/python_packages/qnet_bindings/qnet/`. Optional packaging: `python -m build -w` → wheel including `qoala-opt`/`qoala-translate` binaries. Final consumer: `euqalyptus` importing `qnet.dialects.qnet` and `qnet.ir`.
