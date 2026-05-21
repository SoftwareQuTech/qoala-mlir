# QNet (HIR)

The `qnet` dialect expresses **value-based quantum semantics**. Every quantum operation consumes one or more `!qnet.qubit` SSA values and produces fresh result qubit value(s); there is no explicit memory model. Rotation angles are accepted both as `f32` (the user-facing form emitted by [euqalyptus](https://softwarequtech.github.io/euqalyptus/)) and as integer-pair `(n, e)` encoding `angle = n·π / 2^e`.

Source: `include/Dialect/QNet/{QNet,QNetDialect,QNetOps,QNetTypes}.td`.

## Types

| Type | Mnemonic | Description |
| --- | --- | --- |
| `Qubit` | `!qnet.qubit` | The SSA value type representing a quantum value. |

## Structural ops

### `qnet.func`

Function declaration/definition for qoala programs.

- **Why a custom func op?** It is a clone of `func.func` that also acts as a local symbol table, so `qnet.remote` declarations can live inside a function body and not just at module scope.
- **Traits:** `AffineScope`, `AutomaticAllocationScope`, `FunctionOpInterface`, `IsolatedFromAbove`, `Symbol`.
- **Attributes:** `sym_name`, `function_type`, `sym_visibility?`, `arg_attrs?`, `res_attrs?`.
- **Custom assembly format.**

### `qnet.return`

Function-return terminator for `qnet.func` bodies.

- **Operands:** `Variadic<AnyType>` — the values to return.
- **Traits:** `Pure`, `HasParent<"FuncOp">`, `ReturnLike`, `Terminator`.
- **Format:** `attr-dict ($operands^ ":" type($operands))?`.

### `qnet.remote`

Declares a remote node by name. Acts as a `Symbol` so other ops in the dialect can refer to it via a `FlatSymbolRefAttr`.

- **Attributes:** `sym_name` (remote name), `sym_visibility?`.
- **Format:** `attr-dict $sym_name`.

## Quantum allocation and gates

Each gate consumes the input qubit value(s) and produces a fresh result qubit value (or, for two-qubit gates, two result values). All single-qubit gates carry the `QubitOpIface` trait; the Hermitian gates (`x`, `y`, `z`, `hadamard`, `cnot`, `cz`) also carry `HermitianOpIface`, which the peephole optimizer uses for self-inverse cancellation.

| Op | Operands | Results | Notes |
| --- | --- | --- | --- |
| `qnet.new_qubit` | — | `!qnet.qubit` | Allocates a new qubit value. |
| `qnet.x` | `!qnet.qubit` | `!qnet.qubit` | Hermitian. |
| `qnet.y` | `!qnet.qubit` | `!qnet.qubit` | Hermitian. |
| `qnet.z` | `!qnet.qubit` | `!qnet.qubit` | Hermitian. |
| `qnet.hadamard` | `!qnet.qubit` | `!qnet.qubit` | Hermitian. |
| `qnet.rot_x` | `!qnet.qubit`, `f32` | `!qnet.qubit` | `RotationOpIface`. |
| `qnet.rot_y` | `!qnet.qubit`, `f32` | `!qnet.qubit` | `RotationOpIface`. |
| `qnet.rot_z` | `!qnet.qubit`, `f32` | `!qnet.qubit` | `RotationOpIface`. |
| `qnet.rot_x_int` | `!qnet.qubit`, `i32` n, `i32` e | `!qnet.qubit` | Integer-encoded angle. |
| `qnet.rot_y_int` | `!qnet.qubit`, `i32` n, `i32` e | `!qnet.qubit` | Integer-encoded angle. |
| `qnet.rot_z_int` | `!qnet.qubit`, `i32` n, `i32` e | `!qnet.qubit` | Integer-encoded angle. |
| `qnet.cnot` | 2 × `!qnet.qubit` | 2 × `!qnet.qubit` | Two-qubit, Hermitian. |
| `qnet.cz` | 2 × `!qnet.qubit` | 2 × `!qnet.qubit` | Two-qubit, Hermitian. |
| `qnet.crot_x` | 2 × `!qnet.qubit`, `f32` | 2 × `!qnet.qubit` | Two-qubit rotation, float angle. |
| `qnet.crot_x_int` | 2 × `!qnet.qubit`, `i32` n, `i32` e | 2 × `!qnet.qubit` | Two-qubit rotation, integer-encoded angle. |
| `qnet.measure` | `!qnet.qubit` | `i1` | Standard-basis measurement; the qubit value is consumed. |

## Entanglement

Both ops reference a `qnet.remote` symbol via the `remote` attribute. They carry `UsesRemoteInterface` (verified through the `UsingRemote_Op` mixin).

| Op | Operands / attrs | Results | Notes |
| --- | --- | --- | --- |
| `qnet.eprs` | `remote` (FlatSymbolRefAttr) | `!qnet.qubit` | Generates an entangled qubit value with the remote. |
| `qnet.eprs_measure` | `remote` (FlatSymbolRefAttr) | `i1` | Generates an entangled qubit and measures it; only the classical outcome is materialized. |

## Classical communication

All four pairs are present in single-value and multi-value forms. The multi-value forms use `tensor<…x{i32,f32}>` and exist in HIR mainly so they can be unfolded later (`unfold-comm-ops` at MIR level). (*WIP*: We are exploring solutions to avoid using tensors, since they carry too much complexity for purpose of simply encasulating a set of integers). All carry `UsesRemoteInterface`.

| Op | Operands / attrs | Results |
| --- | --- | --- |
| `qnet.send_int` | `i32`, `remote` | — |
| `qnet.recv_int` | `remote` | `i32` |
| `qnet.send_float` | `f32`, `remote` | — |
| `qnet.recv_float` | `remote` | `f32` |
| `qnet.send_ints` | `tensor<?xi32>`, `remote` | — |
| `qnet.recv_ints` | `remote`, `length: i32` (attr) | `tensor<?xi32>` |
| `qnet.send_floats` | `tensor<?xf32>`, `remote` | — |
| `qnet.recv_floats` | `remote`, `length: i32` (attr) | `tensor<?xf32>` |

## Example

```mlir
module {
  qnet.remote @Bob

  %q1 = qnet.new_qubit : !qnet.qubit
  %ent = qnet.eprs { remote = @Bob } : !qnet.qubit
  %q2, %ent2 = qnet.cnot %q1, %ent : !qnet.qubit, !qnet.qubit
  %q3 = qnet.hadamard %q2 : !qnet.qubit

  %m_local = qnet.measure %q3 : i1
  %m_ent   = qnet.measure %ent2 : i1
}
```

(Adapted from `test/IR/HIR/hir_entangle.mlir` and similar fixtures.)
