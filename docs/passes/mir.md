# MIR helpers

These passes operate on `qmem` modules. Source: `include/Dialect/Helpers/MIRToLIRHelperPasses.td`.

Most are also invoked indirectly by the `lower-qoala-mir-to-lir` wrapper (see [MIR → LIR](mir-to-lir.md)), but they can be invoked individually for testing or finer pipeline control.

## `lower-float-rotations`

Converts every `qmem.rot_x`, `qmem.rot_y`, `qmem.rot_z`, and `qmem.crot_x` op that uses an `f32` angle into its integer-pair `(n, e)` counterpart (`qmem.rot_x_int`, etc.). The new ops encode `angle = n·π / 2^e`.

- **Pass class:** `ConvertIntegerToFloatRotations`. (Note: the class name is reversed relative to what it does — it converts *float* to *int*.)
- **Anchor:** `qmem::FuncOp`.
- **Options:** none.

This pass is **mandatory** before MIR→LIR: NetQASM only accepts the integer-encoded rotation form.

## `unfold-comm-ops`

Unfolds the multi-value classical communication ops (`qmem.send_ints`, `qmem.recv_ints`, `qmem.send_floats`, `qmem.recv_floats`) into sequences of single-value ops (`qmem.send_int`, `qmem.recv_int`, `qmem.send_float`, `qmem.recv_float`). Use `--lower-qoala-mir-to-lir=disable-unfold-comm-ops=true` if you want to keep the multi-value forms (forwarding TODO #72).

- **Pass class:** `UnfoldClassicalCommOps`.
- **Anchor:** `qmem::FuncOp`.
- **Options:** none.

## `functionize`

Extracts groups of contiguous `qmem` quantum operations from the main function into stand-alone `netqasm` routines, replacing each group with a single `qoalahost.call`. Two strategies are available:

- **Per-qubit grouping** (default) — groups operations by the qubits they touch. Bounded by `max-ops-per-group` if non-zero.
- **Simple functionize** (`use-simple-functionize=true`) — wraps **each individual** quantum op in its own routine. PoC-quality; not for production.

- **Pass class:** `FunctionizeQuantumOps`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:**
  - `use-simple-functionize: bool` (default `false`)
  - `max-ops-per-group: uint32_t` (default `0`, meaning "no cap")
- **Dependent dialects:** `qmem`, `func`.

See the [Functionize internals](../internals/functionize.md) page for the algorithm.

## `simple-functionize`

The standalone form of the simple functionize variant, exposed as its own pass. Marked **PoC; DO NOT USE** in the source. Provided for testing.

- **Pass class:** `SimpleFunctionize`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:** none.

## `fold-constants`

Folds constants in nested operations of the module. Wraps the standard MLIR constant-folding facility for use within the qoala pipeline.

- **Pass class:** `FoldConstants`.
- **Anchor:** any op (no anchor restriction).
- **Options:** none.

## `insert-convert-float-angle`

Inserts the declaration of the `__qoala_convert_float_angle` runtime helper into the module. The helper is invoked at runtime when an angle that has to be discretized is not a compile-time constant. Its signature, inserted by the pass, is:

```mlir
netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
```

- **Pass class:** `AngleConversionDeclaration`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:** none.

The pass runs unconditionally (it always inserts the declaration, even if the program doesn't need it), so the IR is always self-consistent after lowering.
