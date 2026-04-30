# QoalaHost (LIR)

The `qoalahost` dialect describes the **classical host body** of an LIR program — the part that runs on the CPS. A `qoalahost.main_func` contains a sequence of blocks, each tagged with `qoalahost.blk_meta`. Quantum work is delegated to NetQASM routines via `qoalahost.call`, and remote nodes are referenced through `qremote.remote` symbols.

Source: `include/Dialect/QoalaHost/{QoalaHost,QoalaHostDialect,QoalaHostOps}.td`.

![QoalaHost block structure](../assets/figures/qoalahost-block-structure.svg)

All ops in this dialect carry the `QuantumOpInterface` trait via the dialect's op base class.

## Structural ops

### `qoalahost.main_func`

The CPS entry point. Hosts the classical program structure.

- **Description:** function wrapper for the operations that are supposed to run on the host machine (CPS).
- **Traits:** `AffineScope`, `AutomaticAllocationScope`, `FunctionOpInterface`, `IsolatedFromAbove`, `Symbol`. Has a region verifier that ensures the body only contains operations from `qoalahost`, `arith`, `memref`, and `cf`.
- **Custom assembly format.**

### `qoalahost.call`

Direct call to a `netqasm.local_routine` or `netqasm.request_routine`.

- **Why a `Terminator`?** Per the iQoala block-shape requirements (one call per QL block), `qoalahost.call` is given the `Terminator` trait. This forces every call to live in its own block, since each MLIR block can only have one terminator.
- **Traits:** `CallOpInterface`, `MemRefsNormalizable`, `SymbolUserOpInterface`, `Terminator`, `QuantumOpInterface`.
- **Operands:** `callee` (`FlatSymbolRefAttr`), `Variadic<AnyType>` arguments.
- **Results:** `Variadic<AnyType>` (the return values of the routine).
- **Format:** `$callee "(" $operands ")" attr-dict ":" functional-type($operands, results)`.

### `qoalahost.return`

Function-return terminator for `qoalahost.main_func` bodies.

- **Traits:** `Pure`, `HasParent<"MainFuncOp">`, `ReturnLike`, `Terminator`.

## Block metadata

### `qoalahost.blk_meta`

A no-op annotation that records per-block metadata used by the runtime and the analyses. Every block in `main_func` is expected to have one.

- **Attributes:**
  - `block_id: StrAttr`
  - `predecessors: StrArrayAttr`
  - `dependencies: StrArrayAttr`
  - `prev_comm: StrAttr`
  - `prev_ent: StrAttr`
  - `deadlines: DictionaryAttr`
- **Trait:** `HasParent<"MainFuncOp">`.

`qoalahost-add-block-precedences` populates `predecessors` and (with `use-online-scheduler=true`) the floater-to-branch dependency edges. `qoalahost-reorder-blocks` reorders blocks and (with `with-deadlines=true`) computes the `deadlines` dictionary.

### `qoalahost.remote_id_ref`

A placeholder operation that records, for a given remote, whether a *classical* and/or *quantum* socket should be reserved. Filled in by the LIR-to-iQoala translation.

- **Attributes:** `remote: FlatSymbolRefAttr`, `classical: BoolAttr`, `quantum: BoolAttr`.
- **Trait:** `HasParent<"MainFuncOp">`.

## Nop terminators

The dialect provides two nop ops to satisfy MLIR's "every block has a terminator" requirement when the natural block content does not include one.

| Op | Trait | Use |
| --- | --- | --- |
| `qoalahost.nop` | — | Mid-block placeholder; no terminator effect. |
| `qoalahost.nop_term` | `Terminator` | Use as the terminator of CL blocks (classical-only blocks that contain no `call`/`recv`). |

## Classical communication

`qoalahost` mirrors the classical send/recv set from QMem, but **all `recv_*` ops are `Terminator`s**, for the same one-recv-per-block reason as `qoalahost.call`. They also implement `ClassicalCommInterface` and `UsesRemoteInterface`.

| Op | Operands / attrs | Results | Trait notes |
| --- | --- | --- | --- |
| `qoalahost.send_int` | `i32`, `remote` | — | — |
| `qoalahost.recv_int` | `remote` | `i32` | `Terminator`. |
| `qoalahost.send_float` | `f32`, `remote` | — | — |
| `qoalahost.recv_float` | `remote` | `f32` | `Terminator`. |
| `qoalahost.send_ints` | `tensor<?xi32>`, `remote` | — | TODO #72: tensor → memref at LIR. |
| `qoalahost.recv_ints` | `remote`, `length: i32` (attr) | `tensor<?xi32>` | `Terminator`. TODO #72. |
| `qoalahost.send_floats` | `tensor<?xf32>`, `remote` | — | TODO #72. |
| `qoalahost.recv_floats` | `remote`, `length: i32` (attr) | `tensor<?xf32>` | `Terminator`. TODO #72. |

## Example

```mlir
module {
  qremote.remote @Bob

  netqasm.local_routine @subrt() -> i1 {
    %v = netqasm.qalloc : i32
    netqasm.init %v
    %m = netqasm.measure %v : i1
    netqasm.return %m : i1
  }

  qoalahost.main_func @main() {
    qoalahost.blk_meta {
      block_id = "block_0", predecessors = [], dependencies = [],
      prev_comm = "", prev_ent = "", deadlines = {}
    }
    qoalahost.remote_id_ref { remote = @Bob, classical = true, quantum = false }
    qoalahost.nop_term

  ^bb1:
    qoalahost.blk_meta {
      block_id = "block_1", predecessors = ["block_0"], dependencies = [],
      prev_comm = "", prev_ent = "", deadlines = {}
    }
    %m = qoalahost.call @subrt() : () -> i1

  ^bb2:
    qoalahost.blk_meta {
      block_id = "block_2", predecessors = ["block_1"], dependencies = [],
      prev_comm = "", prev_ent = "", deadlines = {}
    }
    qoalahost.send_int %m, @Bob : i32
    qoalahost.return
  }
}
```

(Adapted from `test/IR/LIR/lir_local.mlir`.)
