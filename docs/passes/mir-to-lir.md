# MIR → LIR

Source: `include/Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.td`.

## `lower-qoala-mir-to-lir`

The recommended single-pass entry point for MIR → LIR. Internally, it sequences:

1. `functionize` — extract groups of quantum ops into NetQASM routines.
2. `lower-qmem-to-lower-dialects` — rewrite the remaining `qmem` ops to their `qoalahost`, `netqasm`, and `qremote` equivalents.

After this pass:

- The module's main function is a `qoalahost.main_func` with one block per `qoalahost.call`/`recv` boundary.
- Each block carries a `qoalahost.blk_meta` annotation (initially with empty `predecessors`/`dependencies`/`deadlines`; populated later by `qoalahost-add-block-precedences` and `qoalahost-reorder-blocks`).
- Quantum work lives in `netqasm.local_routine` and `netqasm.request_routine` ops.
- Remote-node symbols live as module-scope `qremote.remote` operations.

- **Pass class:** `QoalaMIRToQoalaLIR`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:**
  - `use-simple-functionize: bool` (default `false`) — fall back to the simple, per-op functionize variant. **PoC only.**
  - `use-sccp: bool` (default `false`) — apply SCCP to propagate constants into nested blocks of `main_func` before the lowering.
  - `max-ops-per-group: uint32_t` (default `0`) — cap on operations per per-qubit group.
  - `disable-unfold-comm-ops: bool` (default `false`) — skip the multi-value → single-value unfolding of classical comm ops.
  - `use-online-scheduler: bool` (default `false`) — add conservative floater-to-branch dependency edges on EPR blocks. With the default (`false`), EPR blocks are not forced before branch blocks, so the static scheduler can pipeline EPR generation with Pauli corrections.
- **Dependent dialects:** `func`, `cf`, `qmem`, `qoalahost`, `netqasm`, `qremote`.

!!! warning "Pass options must live inside the pass argument"
    Pass options are passed as part of the pass argument string, not as separate top-level flags. For example:

    ```sh
    qoala-opt program.mlir --lower-qoala-mir-to-lir=use-simple-functionize=true,max-ops-per-group=4
    ```

    `--use-simple-functionize=true` on its own will be rejected by the `cl::opt` parser.

## `lower-qmem-to-lower-dialects`

Lowers all instances of `qmem` operations that can be lowered to LIR dialects (`qoalahost`, `netqasm`, `qremote`). The transformations are bundled into a single pass because the IR is **invalid in between** any of the individual rewrites (e.g. while some `qmem` ops have been replaced and others have not). Run `lower-qoala-mir-to-lir` instead unless you have a specific reason.

- **Pass class:** `LowerQMemToLowerDialects`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:**
  - `use-online-scheduler: bool` (default `false`).
- **Dependent dialects:** `func`, `qmem`, `qoalahost`, `netqasm`, `qremote`.
