# QMem (MIR)

The `qmem` dialect makes the **quantum-memory model explicit**. Qubits are no longer SSA values; they are integer pointers (`i32`) into a quantum-memory bank. Quantum operations therefore become **side-effecting** — they read and write the slot pointed to by their `i32` operand and produce no SSA quantum result. Only `qmem.measure` and `qmem.eprs_measure` produce SSA results, and both return classical `i1` outcomes.

Source: `include/Dialect/QMem/{QMem,QMemDialect,QMemOps}.td`.

![QMem memory model](../assets/figures/qmem-memory-model.svg)

All ops in this dialect carry the `SimpleCloneInterface` and `OpQubitsInterface` traits via the dialect's op base class.

## Structural ops

### `qmem.func`

Function declaration/definition for qoala programs. Same shape as `qnet.func`.

- **Traits:** `AffineScope`, `AutomaticAllocationScope`, `FunctionOpInterface`, `IsolatedFromAbove`, `Symbol`.
- **Custom assembly format.**

### `qmem.return`

Function-return terminator for `qmem.func` bodies.

- **Traits:** `Pure`, `HasParent<"FuncOp">`, `ReturnLike`, `Terminator`.

### `qmem.remote`

Module-scope remote-name symbol. Same shape as `qnet.remote`.

## Quantum-memory ops

| Op | Operands | Results | Notes |
| --- | --- | --- | --- |
| `qmem.qalloc` | — | `i32` | Allocates a fresh qubit slot and returns its pointer. |
| `qmem.init` | `i32` qubit | — | Initializes the slot. `DefineQubitsInterface`. |
| `qmem.measure` | `i32` qubit | `i1` | Standard-basis measurement; mutates the slot via `MemWrite`. |

## Quantum gates

All gates take qubit pointers (`i32`) annotated with the `MemWrite` memory-effect; they produce no result.

### Float-angle rotations

| Op | Operands |
| --- | --- |
| `qmem.rot_x` | `i32` qubit, `f32` angle |
| `qmem.rot_y` | `i32` qubit, `f32` angle |
| `qmem.rot_z` | `i32` qubit, `f32` angle |
| `qmem.crot_x` | 2 × `i32` qubit, `f32` angle |

### Integer-encoded rotations (`angle = n·π / 2^e`)

| Op | Operands |
| --- | --- |
| `qmem.rot_x_int` | `i32` qubit, `i32` n, `i32` e |
| `qmem.rot_y_int` | `i32` qubit, `i32` n, `i32` e |
| `qmem.rot_z_int` | `i32` qubit, `i32` n, `i32` e |
| `qmem.crot_x_int` | 2 × `i32` qubit, `i32` n, `i32` e |

The `lower-float-rotations` pass rewrites every float-angle rotation in the table above to its integer-encoded counterpart, since NetQASM only accepts the integer form.

### Discrete gates

| Op | Operands |
| --- | --- |
| `qmem.x`, `qmem.y`, `qmem.z`, `qmem.hadamard` | `i32` qubit |
| `qmem.cnot`, `qmem.cz` | 2 × `i32` qubit |

## Entanglement

Both carry the `Entangle` native trait, plus `DefineQubitsInterface`, `EntangleInterface`, and `UsesRemoteInterface`.

| Op | Operands / attrs | Results |
| --- | --- | --- |
| `qmem.eprs` | `i32` qubit (`MemWrite`), `remote` | — |
| `qmem.eprs_measure` | `i32` qubit (`MemWrite`), `remote` | `i1` |

## Classical communication

Same eight ops as in QNet (single-value and multi-value forms). All carry `UsesRemoteInterface`. The MIR-level `unfold-comm-ops` pass rewrites the multi-value forms (`send_ints`, `recv_ints`, `send_floats`, `recv_floats`) into sequences of single-value calls.

| Op | Operands / attrs | Results |
| --- | --- | --- |
| `qmem.send_int` | `i32`, `remote` | — |
| `qmem.recv_int` | `remote` | `i32` |
| `qmem.send_float` | `f32`, `remote` | — |
| `qmem.recv_float` | `remote` | `f32` |
| `qmem.send_ints` | `tensor<?xi32>`, `remote` | — |
| `qmem.recv_ints` | `remote`, `length: i32` (attr) | `tensor<?xi32>` |
| `qmem.send_floats` | `tensor<?xf32>`, `remote` | — |
| `qmem.recv_floats` | `remote`, `length: i32` (attr) | `tensor<?xf32>` |

## Example

```mlir
module {
  qmem.remote @Bob

  %qptr = qmem.qalloc : i32
  qmem.init %qptr

  %f = qmem.recv_float { remote = @Bob } : f32
  qmem.rot_x %qptr, %f
  qmem.hadamard %qptr

  %m = qmem.measure %qptr : i1
}
```

(Adapted from `test/IR/MIR/mir_local.mlir`.)
