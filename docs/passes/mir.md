# MIR helpers

These passes operate on `qmem` modules. Source: `include/Dialect/Helpers/MIRToLIRHelperPasses.td`.

Most of them are also invoked indirectly by the `lower-qoala-mir-to-lir` wrapper (see [MIR → LIR](mir-to-lir.md)), but each one can be invoked individually for testing or finer pipeline control.

## `lower-float-rotations`

Converts every `qmem.rot_x`, `qmem.rot_y`, `qmem.rot_z`, and `qmem.crot_x` op that takes an `f32` angle into its integer-pair `(n, e)` counterpart (`qmem.rot_x_int`, and so on), encoding the angle as `n·π / 2^e`. This is a mandatory pass before MIR-to-LIR: NetQASM only accepts the integer-encoded rotation form, so any float-angle rotation that survives this pass would later be rejected. Note that the C++ class name is `ConvertIntegerToFloatRotations`, which is reversed relative to what it actually does — the pass converts float-angle rotations to integer-angle ones.

- **Pass class:** `ConvertIntegerToFloatRotations`.
- **Anchor:** `qmem::FuncOp`.
- **Options:** none.

## `unfold-comm-ops`

Unfolds the multi-value classical communication ops (`qmem.send_ints`, `qmem.recv_ints`, `qmem.send_floats`, `qmem.recv_floats`) into sequences of their single-value variants (`qmem.send_int`, `qmem.recv_int`, `qmem.send_float`, `qmem.recv_float`). This is what lets each individual classical send or receive be isolated into its own block during the downstream functionization step. If you want to keep the multi-value forms (the long-term plan is to replace the tensor representation with `memref` for simplicify reasons), use `--lower-qoala-mir-to-lir=disable-unfold-comm-ops=true`.

- **Pass class:** `UnfoldClassicalCommOps`.
- **Anchor:** `qmem::FuncOp`.
- **Options:** none.

## `functionize`

Extracts groups of contiguous `qmem` quantum operations from the main function into stand-alone `netqasm` routines, replacing each group with a single `qoalahost.call`. The default strategy groups operations by the qubits they touch (with the size of each group optionally capped via `max-ops-per-group`); the alternative strategy, enabled by `use-simple-functionize=true`, wraps each individual quantum op in its own routine. The simple variant is proof-of-concept quality and not intended for production use. See the [Functionize internals](../internals/functionize.md) page for the full algorithm.

- **Pass class:** `FunctionizeQuantumOps`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:**
    - `use-simple-functionize: bool` (default `false`).
    - `max-ops-per-group: uint32_t` (default `0`, meaning "no cap").
- **Dependent dialects:** `qmem`, `func`.

## `simple-functionize`

The standalone form of the simple functionize variant, exposed as its own pass for testing. It is marked **PoC; DO NOT USE** in the source — it exists only to make the variant invokable on its own.

- **Pass class:** `SimpleFunctionize`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:** none.

## `fold-constants`

Folds constants in nested operations of the module by wrapping the standard MLIR constant-folding facility for use within the qoala pipeline. The pass has no anchor restriction and no options.

- **Pass class:** `FoldConstants`.
- **Anchor:** any op (no anchor restriction).
- **Options:** none.

## `insert-convert-float-angle`

Inserts the declaration of the `__qoala_convert_float_angle` runtime helper into the module. This helper is invoked at runtime whenever an angle to be discretized is not a compile-time constant. The declaration the pass inserts is:

```mlir
netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
```

The pass runs unconditionally — it always inserts the declaration, even when the program does not end up needing it — so the IR is always self-consistent after lowering.

- **Pass class:** `AngleConversionDeclaration`.
- **Anchor:** `mlir::ModuleOp`.
- **Options:** none.
