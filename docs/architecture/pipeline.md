# Pass pipeline

`qoala-opt` does not enforce a fixed pipeline: the user composes passes via `--pass-name=…` arguments, exactly like upstream `mlir-opt`. This page documents the orderings that are known to be correct and that the toolchain is built around.

## End-to-end recommended order

Given an HIR module `program.mlir`, the canonical lowering to LIR is:

```sh
qoala-opt program.mlir \
  --qnet-peephole-optimizations \
  --qnet-dead-code-elimination \
  --lower-qoala-hir-to-mir \
  --lower-qoala-mir-to-lir
```

This command expands into five logical stages, the last two of which are bundled by the wrapper passes.

The first stage runs the **HIR-level optimizations**. These are optional but recommended. The verifier `qnet-check-linear` should run before any rewrite that assumes linearity. `qnet-peephole-optimizations` performs Hermitian cancellation and rotation folding by default; sub-options for the additional Pauli-to-rotation conversion and full-turn elimination are documented in [Passes / HIR](../passes/hir.md). `qnet-dead-code-elimination` then strips quantum dataflow whose results never reach an observable measurement; toggling `with-classical-awareness=true` further scopes DCE to measurements whose outcomes are actually consumed by the classical program.

The second stage is the **HIR-to-MIR lowering**. `lower-qoala-hir-to-mir` performs the full QNet-to-QMem conversion, introducing explicit `i32` qubit pointers and turning quantum ops into side-effecting calls on those pointers.

The third stage is the set of **MIR helpers**. These are run by `lower-qoala-mir-to-lir` internally but can also be invoked individually. `lower-float-rotations` converts `f32`-angle rotations to their integer-pair `(n, e)` form (NetQASM only accepts the integer-encoded angles). `unfold-comm-ops` splits multi-value classical communication ops into single-value variants; you can disable it with `--lower-qoala-mir-to-lir=disable-unfold-comm-ops=true`. `fold-constants` is generic constant folding inside nested ops. `functionize` extracts groups of contiguous quantum ops into NetQASM routines; its `max-ops-per-group` option caps the maximum number of quantum operations per routine. `insert-convert-float-angle` injects the `__qoala_convert_float_angle` runtime declaration whenever runtime angle discretization is needed.

The fourth stage is the **MIR-to-LIR conversion**. The recommended way to run it is `lower-qoala-mir-to-lir`, which wraps `functionize` and `lower-qmem-to-lower-dialects` into a single pass invocation. The wrapper exposes a handful of options that propagate inwards: `use-simple-functionize=true` falls back to the per-op simple functionize (a proof-of-concept variant, not for production); `use-sccp=true` applies SCCP to propagate constants into nested blocks of `main_func` before the lowering; `max-ops-per-group` caps the size of per-qubit op groups when functionizing; `disable-unfold-comm-ops=true` skips the multi-value-to-single-value unfolding; and `use-online-scheduler=true` adds conservative floater-to-branch dependencies on EPR (QC) blocks. The default (`use-online-scheduler=false`) lets the static scheduler pipeline EPR generation with Pauli corrections.

The fifth stage is **LIR finalization**, which is run as a separate set of passes. `qoalahost-add-block-precedences` materializes the predecessor edges into `qoalahost.blk_meta`; it honors `use-online-scheduler`. `qoalahost-reorder-blocks` then performs the MILP-driven qubit-lifetime reduction; setting `with-deadlines=true` also computes and records per-block deadlines into the same `blk_meta` dictionary. Finally, the `qoalahost-show-analysis-*` family of passes prints the corresponding analyses to stdout (see [Passes / LIR](../passes/lir.md)).

The pipeline ends with translation:

```sh
qoala-translate --mlir-to-iqoala lowered.mlir > program.iqoala
```

## Mandatory vs. optional

Several passes are mandatory for the resulting LIR to be valid. `lower-qoala-hir-to-mir` is required because nothing downstream can consume HIR. `lower-float-rotations` is required because NetQASM cannot consume `f32` angles. `lower-qoala-mir-to-lir` (or its constituent `functionize` plus `lower-qmem-to-lower-dialects`) is required because nothing downstream can consume MIR. And `qoalahost-add-block-precedences` is required because the runtime expects `blk_meta` to be populated.

The remaining passes are optimizations and can be safely skipped if all you want is a correct LIR. This applies to `qnet-peephole-optimizations`, `qnet-dead-code-elimination`, `qoalahost-reorder-blocks`, `fold-constants`, and the `*-show-analysis-*` family (which are observers — they never transform the IR).

## Cost-model flags

The `--qoala-opt-*` cost-model flags (`--qoala-opt-single-gate-duration`, `--qoala-opt-link-duration`, `--qoala-opt-program-horizon`, …) feed the analysis-print passes (`gate-count`, `qubit-life`, `qmem-eff`, `esp`) and the MILP solver inside `qoalahost-reorder-blocks`. They have no effect on lowering correctness — only on the numbers reported by the analyses and on the MILP objective. The full table lives in [tools/qoala-opt](../tools/qoala-opt.md).

## Standard MLIR debugging knobs

`qoala-opt` exposes the standard `mlir-opt` debugging flags unchanged. `--debug` and `--debug-only=<filter>` enable `LLVM_DEBUG` prints, optionally filtered by `DEBUG_TYPE`. `--mlir-print-op-generic` and `--mlir-print-assume-verified` help debug custom verifiers. `--mlir-disable-threading` serializes pass execution, which is necessary to keep debug output ordered. `--mlir-print-value-users` annotates the printed IR with value-user comments. `--print-ir-after=<pass>`, `--print-ir-before=<pass>`, `--print-after-all`, and `--print-before-all` dump the IR around specific passes. `--view-op-graph` prints a Graphviz representation of the op tree. All of these flow through unchanged from `MlirOptMain`.
