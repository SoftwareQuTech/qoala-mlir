# The three IRs

`qoala-mlir` lowers programs through three intermediate representations: HIR, expressed in the `qnet` dialect; MIR, expressed in the `qmem` dialect; and LIR, expressed jointly in the `qoalahost`, `netqasm`, and `qremote` dialects. The three levels describe the same program at progressively more concrete levels of abstraction, and a given pass is meaningful at exactly one of them.

![Dialect graph](../assets/figures/dialect-graph.svg)

## HIR — `qnet`

HIR follows the Static Single Assignment (SSA) paradigm for quantum data: every quantum gate takes one or more `!qnet.qubit` values as operands and produces fresh `!qnet.qubit` result values that represent the post-operation state of those qubits. The qubit is never mutated in place; it is replaced, at the SSA level, by a new value. There is no explicit memory model in HIR — qubits exist only as SSA values flowing through the program — and communication and entanglement ops reference remote nodes by symbol (`qnet.remote`).

```mlir
%q1 = qnet.new_qubit : !qnet.qubit
%q2 = qnet.rot_x %q1, %angle : !qnet.qubit
%q3 = qnet.hadamard %q2 : !qnet.qubit
%m  = qnet.measure %q3 : i1
```

This shape makes algebraic rewrites on quantum data structurally natural. Hermitian cancellation and rotation folding are local def-use traversals on `!qnet.qubit` chains, dead-code elimination is backward liveness propagation from measurements, and the linear-types invariant — that each `!qnet.qubit` value has at most one consumer — is a single-pass check. The three HIR-level passes that exploit this are `qnet-peephole-optimizations`, which performs Hermitian cancellation, rotation folding, and optional Pauli-to-rotation conversion; `qnet-dead-code-elimination`, which drops dataflow whose quantum results never reach an observable measurement (with `with-classical-awareness=true`, only measurements whose classical outcomes are observably consumed count as roots); and `qnet-check-linear`, the single-use verifier.

See the full op list in [Operations reference / QNet](../reference/qnet.md) and the pass details in [Passes / HIR](../passes/hir.md).

## MIR — `qmem`

The HIR→MIR lowering (`lower-qoala-hir-to-mir`) replaces every `!qnet.qubit` SSA value with an explicit `i32` qubit pointer, allocated by `qmem.qalloc` and initialized by `qmem.init`. Once that rewrite is done, quantum operations are no longer value-to-value: they become side-effecting instructions that take and write to `i32` pointers and produce no SSA quantum result. Only operations that yield classical data — `qmem.measure` and `qmem.eprs_measure` — still produce SSA results.

```mlir
%qptr = qmem.qalloc : i32
qmem.init %qptr
qmem.rot_x %qptr, %angle
qmem.hadamard %qptr
%m = qmem.measure %qptr : i1
```

MIR is where the program is prepared for the structural changes that turn it into a Qoala-block-shaped LIR. Floating-point rotation angles are still permitted, but the `lower-float-rotations` pass rewrites them to the integer-pair form `qmem.rot_*_int` (where the rotation angle is `n·π / 2^e`), the only form NetQASM accepts. Multi-value classical communication ops such as `qmem.send_ints` and `qmem.recv_floats` are decomposed into sequences of their single-value variants by `unfold-comm-ops`, so they can later be isolated into their own host blocks without functionization needing to special-case the tensor forms. And `functionize` extracts contiguous groups of quantum operations into stand-alone NetQASM routines, replacing each group with a single `qoalahost.call`. By the end of MIR processing, every quantum operation is either inside a routine or about to be.

See [Operations reference / QMem](../reference/qmem.md) and [Passes / MIR helpers](../passes/mir.md).

![QMem memory model](../assets/figures/qmem-memory-model.svg)

## LIR — `qoalahost` + `netqasm` + `qremote`

The MIR→LIR lowering (`lower-qoala-mir-to-lir`, which itself sequences `functionize`, the dialect mapping, and the precedence-annotation pass) splits the single mixed function into three layers that mirror the iQoala executable. The classical host body lives in a `qoalahost.main_func`, structured as a chain of MLIR basic blocks. Each block carries a `qoalahost.blk_meta` annotation that records its block ID, control-flow predecessors, data and side-effect dependencies, the immediately preceding communication or request-routine block (if any), and the deadlines computed during reordering. Classical computation, classical sends, classical receives, and calls into quantum routines live directly inside these blocks. The quantum routines themselves are extracted into one or more `netqasm.local_routine` and `netqasm.request_routine` operations, each called from the host body via `qoalahost.call`. Finally, every remote node referenced by `qoalahost.send_*`, `qoalahost.recv_*`, `netqasm.eprs`, or similar is declared once at module scope as a `qremote.remote @<name>` symbol.

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

LIR is where the runtime-facing structure of the program becomes visible and where the scheduling-oriented passes operate. `qoalahost-add-block-precedences` materializes the block-ordering edges required by the runtime — control-flow edges, SSA data dependencies, classical-communication ordering, request-routine ordering, and conservative qubit-side-effect ordering — into the `blk_meta` annotations. `qoalahost-reorder-blocks` then solves a MILP that picks a block permutation minimizing total qubit lifetime, optionally computing per-block deadlines that are also recorded in `blk_meta` when invoked with `with-deadlines=true`. The analysis-print passes — `qoalahost-show-analysis-qmem-eff`, `qubit-life`, `gate-count`, and `esp` — read the same structure to report static metrics. Once the module is finalized, `qoala-translate --mlir-to-iqoala` walks it once more to emit the `.iqoala` executable.

See [Operations reference / QoalaHost](../reference/qoalahost.md), [/ NetQASM](../reference/netqasm.md), [/ QRemote](../reference/qremote.md), and [Passes / LIR](../passes/lir.md).

![QoalaHost block structure](../assets/figures/qoalahost-block-structure.svg)

## Single-pass shortcuts

Two convenience passes wrap whole conversion stages. `lower-qoala-hir-to-mir` runs the full HIR→MIR conversion as a single pass; `lower-qoala-mir-to-lir` does the same for MIR→LIR, internally sequencing `functionize`, the dialect mapping, and the precedence-annotation pass. Options of the inner passes are exposed as pass options of the wrapper: for instance, `--lower-qoala-mir-to-lir=use-simple-functionize=true,max-ops-per-group=5` forwards both options to the inner `functionize` pass. Note that when invoking a pass option from `qoala-opt` you need to specify the option *inside* the pass argument — `--lower-qoala-mir-to-lir=use-online-scheduler=true` works, but `--lower-qoala-mir-to-lir --use-online-scheduler=true` does not.
