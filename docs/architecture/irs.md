# The three IRs

`qoala-mlir` lowers programs through three intermediate representations:

- **HIR** — the `qnet` dialect.
- **MIR** — the `qmem` dialect.
- **LIR** — the `qoalahost` dialect for the classical host body, plus the `netqasm` dialect for quantum routines and the `qremote` dialect for remote-node symbols.

Each level describes the same program at a different level of abstraction. A given pass is meaningful at exactly one level.

![Dialect graph](../assets/figures/dialect-graph.svg)

## HIR — `qnet`

In HIR a quantum program is expressed as **value-to-value transformations on `!qnet.qubit` SSA values**. There is no explicit memory model: every gate consumes its qubit operand(s) and produces fresh result qubit value(s). Communication and entanglement ops reference remote nodes by symbol (`qnet.remote`).

```mlir
%q1 = qnet.new_qubit : !qnet.qubit
%q2 = qnet.rot_x %q1, %angle : !qnet.qubit
%q3 = qnet.hadamard %q2 : !qnet.qubit
%m  = qnet.measure %q3 : i1
```

This construction makes some algebraic rewrites (gate cancellation, rotation merging, dead-code elimination) straightforward. Some use cases:

- `qnet-peephole-optimizations` — Hermitian cancellation, rotation folding, optional Pauli→rotation conversion.
- `qnet-dead-code-elimination` — drop quantum dataflow that has no observable effect; with `with-classical-awareness=true`, only measurements whose classical outcome is observably used count as liveness roots.
- `qnet-check-linear` — verify that each `!qnet.qubit` value is used at most once (a standard linear-types invariant).

See the full op list in [Operations reference / QNet](../reference/qnet.md) and the pass details in [Passes / HIR](../passes/hir.md).

## MIR — `qmem`

The HIR→MIR lowering (`lower-qoala-hir-to-mir`) rewrites every `!qnet.qubit` SSA value to an explicit **`i32` qubit pointer**, allocated by `qmem.qalloc` and initialized by `qmem.init`. Quantum operations become **side-effecting** (they take and write to `i32` pointers and produce no SSA quantum result):

```mlir
%qptr = qmem.qalloc : i32
qmem.init %qptr
qmem.rot_x %qptr, %angle
qmem.hadamard %qptr
%m = qmem.measure %qptr : i1
```

MIR is the level at which:

- Floating-point rotation angles can still appear, but `lower-float-rotations` rewrites them to `qmem.rot_*_int` ops which use an integer-pair `(n, e)` form  where `angle = n·π / 2^e`. This matches NetQASM's allowed encoding.
- `unfold-comm-ops` decomposes multi-value classical communication (`qmem.send_ints`, `qmem.recv_floats`) into single-value variants.
- `functionize` extracts contiguous groups of quantum ops into stand-alone NetQASM routines, replacing each group with a single `qoalahost.call`.

See [Operations reference / QMem](../reference/qmem.md) and [Passes / MIR helpers](../passes/mir.md).

![QMem memory model](../assets/figures/qmem-memory-model.svg)

## LIR — `qoalahost` + `netqasm` + `qremote`

The MIR→LIR lowering (`lower-qoala-mir-to-lir`, which itself sequences `functionize` then `lower-qmem-to-lower-dialects`) splits the single mixed function into:

- A `qoalahost.main_func` made of blocks. Each block carries a `qoalahost.blk_meta` annotation that records its block id, predecessors, dependencies, and (optionally) deadlines. Classical and communication operations live directly in these blocks.
- One or more `netqasm.local_routine` and `netqasm.request_routine` operations containing the quantum work, called from the host body via `qoalahost.call`.
- `qremote.remote @<name>` symbols at module scope for every remote node referenced by `qoalahost.send_*`, `qoalahost.recv_*`, `netqasm.eprs`, etc.

```mlir
qremote.remote @Bob

netqasm.local_routine @subrt1() -> i32 {
  %v = netqasm.qalloc : i32
  netqasm.init %v
  netqasm.return %v : i32
}

qoalahost.main_func @main() {
  qoalahost.blk_meta { block_id = "block_0", ... }
  qoalahost.remote_id_ref { classical = false, quantum = true, remote = @Bob }
  qoalahost.nop_term
^bb1:
  qoalahost.blk_meta { block_id = "block_1", ... }
  %v = qoalahost.call @subrt1() : () -> i32
  ...
}
```

This is the level at which:

- `qoalahost-add-block-precedences` materializes the block-ordering edges required by the runtime.
- `qoalahost-reorder-blocks` solves a MILP to shorten qubit lifetimes; with `with-deadlines=true`, deadlines are computed and recorded in `blk_meta`.
- The analysis-print passes (`qoalahost-show-analysis-{qmem-eff,qubit-life,gate-count,esp}`) report on the program.
- Once finalized, `qoala-translate --mlir-to-iqoala` emits the `.iqoala` executable.

See [Operations reference / QoalaHost](../reference/qoalahost.md), [/ NetQASM](../reference/netqasm.md), [/ QRemote](../reference/qremote.md), and [Passes / LIR](../passes/lir.md).

![QoalaHost block structure](../assets/figures/qoalahost-block-structure.svg)

## Single-pass shortcuts

Two convenience passes wrap whole stages:

- `lower-qoala-hir-to-mir` — runs the full HIR→MIR conversion.
- `lower-qoala-mir-to-lir` — runs the full MIR→LIR conversion (functionize + lower-qmem-to-lower-dialects). Options of inner passes are exposed as pass options of this wrapper, e.g. `--lower-qoala-mir-to-lir=use-simple-functionize=true,max-ops-per-group=5`.

Note: when invoking a pass option from `qoala-opt`, you need to specify the option **inside** the pass argument, not as a separate flag. For example, `--lower-qoala-mir-to-lir=use-online-scheduler=true` works, but `--lower-qoala-mir-to-lir --use-online-scheduler=true` does not.
