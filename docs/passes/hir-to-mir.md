# HIR → MIR

Source: `include/Conversion/QoalaHIRToQoalaMIR/QoalaHIRToQoalaMIR.td`.

## `lower-qoala-hir-to-mir`

Lowers all instances of `qnet` dialect ops to `qmem`. After this pass, every `!qnet.qubit` SSA value has been replaced by an `i32` qubit pointer allocated by `qmem.qalloc` and initialized by `qmem.init`. Quantum gates become side-effecting calls on those pointers.

- **Pass class:** `QoalaHIRToQoalaMIR`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:** none.
- **Dependent dialects:** `qnet`, `qmem`, `scf`.

This is a **mandatory** pass for any program that needs to reach LIR.

## What changes during the lowering

- Each `qnet.new_qubit` → `qmem.qalloc` + `qmem.init`.
- Each gate op (e.g. `%q' = qnet.rot_x %q, %a`) → side-effecting equivalent (`qmem.rot_x %qptr, %a`); the SSA result chain is collapsed to the qubit pointer.
- `qnet.measure` → `qmem.measure`.
- `qnet.eprs` / `qnet.eprs_measure` → `qmem.eprs` / `qmem.eprs_measure`.
- All `qnet.send_*` / `qnet.recv_*` ops → their `qmem.*` counterparts (kept in multi-value form at this stage; `unfold-comm-ops` at MIR level rewrites them to single-value).
- `qnet.remote` → `qmem.remote`.
- `qnet.func` → `qmem.func`.

After this pass the program is valid `qmem` IR but is still anchored at `qmem.func`. Subsequent MIR passes (`lower-float-rotations`, `unfold-comm-ops`, `functionize`, `fold-constants`, `insert-convert-float-angle`) prepare it for the MIR→LIR conversion.
