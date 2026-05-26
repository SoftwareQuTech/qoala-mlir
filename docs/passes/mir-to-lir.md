# MIR → LIR

Source: `include/Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIR.td`.

## `lower-qoala-mir-to-lir`

The recommended single-pass entry point for the MIR-to-LIR conversion. Internally it sequences `functionize` (which extracts groups of quantum ops into NetQASM routines) and `lower-qmem-to-lower-dialects` (which rewrites the remaining `qmem` ops into their `qoalahost`, `netqasm`, and `qremote` equivalents). After the pass runs, the module's main function is a `qoalahost.main_func` with one block per `qoalahost.call`/`recv` boundary, each block carrying a `qoalahost.blk_meta` annotation (initially with empty `predecessors` / `dependencies` / `deadlines` — populated later by `qoalahost-add-block-precedences` and `qoalahost-reorder-blocks`). Quantum work lives in `netqasm.local_routine` and `netqasm.request_routine` ops, and remote-node symbols live as module-scope `qremote.remote` operations.

- **Pass class:** `QoalaMIRToQoalaLIR`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:**
    - `use-simple-functionize: bool` (default `false`) — fall back to the simple, per-op functionize variant. **PoC only.**
    - `use-sccp: bool` (default `false`) — apply SCCP to propagate constants into nested blocks of `main_func` before the lowering.
    - `max-ops-per-group: uint32_t` (default `0`) — cap on operations per per-qubit group.
    - `disable-unfold-comm-ops: bool` (default `false`) — skip the multi-value to single-value unfolding of classical comm ops.
    - `use-online-scheduler: bool` (default `false`) — add conservative floater-to-branch dependency edges on EPR blocks. With the default (`false`), EPR blocks are not forced before branch blocks, so the static scheduler can pipeline EPR generation with Pauli corrections.
- **Dependent dialects:** `func`, `cf`, `qmem`, `qoalahost`, `netqasm`, `qremote`.

!!! warning "Pass options must live inside the pass argument"
    Pass options are passed as part of the pass argument string, not as separate top-level flags. For example:

    ```sh
    qoala-opt program.mlir --lower-qoala-mir-to-lir=use-simple-functionize=true,max-ops-per-group=4
    ```

    `--use-simple-functionize=true` on its own will be rejected by the `cl::opt` parser.

## `lower-qmem-to-lower-dialects`

Lowers all instances of `qmem` operations that can be lowered to LIR dialects (`qoalahost`, `netqasm`, `qremote`). The transformations are bundled into a single pass because the IR is invalid in between any two of the individual rewrites — once one `qmem` op has been replaced and another has not, the module is in an inconsistent state and downstream passes can crash. Run `lower-qoala-mir-to-lir` instead unless you have a specific reason to invoke this pass directly.

- **Pass class:** `LowerQMemToLowerDialects`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:**
    - `use-online-scheduler: bool` (default `false`).
- **Dependent dialects:** `func`, `qmem`, `qoalahost`, `netqasm`, `qremote`.
