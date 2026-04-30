# NetQASM (LIR)

The `netqasm` dialect describes the **quantum routines** that run on the QPS. A routine is a `netqasm.local_routine` or `netqasm.request_routine` — both are function-like ops with their own qubit-pointer arguments and (optionally) classical return values. Quantum gates are NetQASM-shaped: only the integer-encoded rotation form (`angle = n·π / 2^e`) is allowed.

Source: `include/Dialect/NetQASM/{NetQASM,NetQASMDialect,NetQASMOps}.td`.

![NetQASM routine](../assets/figures/netqasm-routine.svg)

All ops in this dialect carry the `QuantumOpInterface` trait (and most carry `getDuration`).

## Routines

### `netqasm.local_routine`

Function-like op containing local-only quantum work that runs on the QPS.

- **Traits:** `AffineScope`, `AutomaticAllocationScope`, `FunctionOpInterface`, `IsolatedFromAbove`, `Symbol`, `NetQASMRoutineInterface`.
- **Region verifier:** the body may contain only ops from the dialects allowed by `NetQASMDialect` (the standard `arith`, `memref`, `cf`, plus `netqasm` itself; full list defined in `Dialect/NetQASM/NetQASM.h`).
- **Custom assembly format.**

### `netqasm.request_routine`

Function-like op describing a routine that performs an entanglement request with a remote node. Same traits and region verifier as `netqasm.local_routine`.

### `netqasm.return`

Function-return terminator for both routine kinds.

- **Traits:** `Pure`, `ParentOneOf<["LocalRoutineOp", "RequestRoutineOp"]>`, `ReturnLike`, `Terminator`.

## Quantum-memory ops

| Op | Operands | Results | Notes |
| --- | --- | --- | --- |
| `netqasm.qalloc` | — | `i32` | Allocates a fresh qubit slot. |
| `netqasm.init` | `i32` qubit (`MemWrite`) | — | Initializes the slot. |
| `netqasm.qfree` | `i32` qubit | — | Frees the slot back to the qubit pool. |

## Quantum gates

All gates take `i32` qubit pointers (`MemWrite`-annotated) and produce no results. Rotations only accept the **integer-encoded angle** form: NetQASM does not support `f32` operands here.

### Rotations (`angle = n·π / 2^e`)

| Op | Operands |
| --- | --- |
| `netqasm.rot_x` | `i32` qubit, `i32` n, `i32` e |
| `netqasm.rot_y` | `i32` qubit, `i32` n, `i32` e |
| `netqasm.rot_z` | `i32` qubit, `i32` n, `i32` e |
| `netqasm.crot_x` | 2 × `i32` qubit, `i32` n, `i32` e |

### Discrete gates

| Op | Operands |
| --- | --- |
| `netqasm.x`, `netqasm.y`, `netqasm.z`, `netqasm.hadamard` | `i32` qubit |
| `netqasm.cnot`, `netqasm.cz` | 2 × `i32` qubit |

### Measurement

| Op | Operands | Results |
| --- | --- | --- |
| `netqasm.measure` | `i32` qubit (`MemWrite`) | `i1` |

## Entanglement

Both ops reference a `qremote.remote` symbol via `remote: FlatSymbolRefAttr`, validated by `UsesRemoteInterface`. They appear inside `netqasm.request_routine` bodies.

| Op | Operands / attrs | Results | Notes |
| --- | --- | --- | --- |
| `netqasm.eprs` | `i32` qubit (`MemWrite`), `remote` | — | Generates one entangled qubit. |
| `netqasm.eprs_measure` | `i32` qubit (`MemWrite`), `remote` | `i1` | Generates and measures the entangled qubit; only the classical outcome is returned. |

## Example

```mlir
netqasm.local_routine @subrt(%q: i32, %n: i32, %e: i32) -> i1 {
  netqasm.rot_x %q, %n, %e
  netqasm.hadamard %q
  %m = netqasm.measure %q : i1
  netqasm.return %m : i1
}

netqasm.request_routine @epr_with_bob(%q: i32) -> i1 {
  netqasm.eprs_measure %q, @Bob : i1
}
```

(Adapted from `test/IR/LIR/lir_local.mlir` and `lir_entangle.mlir`.)
