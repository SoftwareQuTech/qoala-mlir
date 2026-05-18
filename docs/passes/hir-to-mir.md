# HIR → MIR

Source: `include/Conversion/QoalaHIRToQoalaMIR/QoalaHIRToQoalaMIR.td`.

## `lower-qoala-hir-to-mir`

Lowers all instances of `qnet` dialect ops to their `qmem` counterparts. After this pass runs, every `!qnet.qubit` SSA value has been replaced by an `i32` qubit pointer allocated by `qmem.qalloc` and initialized by `qmem.init`, and quantum gates become side-effecting calls on those pointers — no SSA quantum result threads through them any more. This is a mandatory pass for any program that needs to reach LIR; the rest of the toolchain expects MIR (or lower) as input.

- **Pass class:** `QoalaHIRToQoalaMIR`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:** none.
- **Dependent dialects:** `qnet`, `qmem`, `scf`.

## What changes during the lowering

Each `qnet.new_qubit` is rewritten to a `qmem.qalloc` followed by a `qmem.init` on the resulting pointer. Each gate op (`%q' = qnet.rot_x %q, %a`, and similar) is rewritten to its side-effecting equivalent (`qmem.rot_x %qptr, %a`), and the SSA result chain that threaded through `!qnet.qubit` values collapses to a single qubit pointer per logical qubit. `qnet.measure` becomes `qmem.measure`; `qnet.eprs` and `qnet.eprs_measure` become their `qmem.*` counterparts; classical `qnet.send_*` and `qnet.recv_*` ops become the corresponding `qmem.*` ops, still in multi-value form at this stage (the `unfold-comm-ops` MIR helper rewrites them to single-value later). The module-scope `qnet.remote` declarations become `qmem.remote`, and `qnet.func` becomes `qmem.func`.

After the pass runs, the program is valid `qmem` IR but is still anchored at `qmem.func`. The subsequent MIR helpers — `lower-float-rotations`, `unfold-comm-ops`, `functionize`, `fold-constants`, `insert-convert-float-angle` — prepare the program for the MIR-to-LIR conversion.
