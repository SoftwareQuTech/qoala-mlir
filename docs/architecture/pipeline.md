# Pass pipeline

`qoala-opt` does not enforce a fixed pipeline: the user composes passes via `--pass-name=…` arguments, exactly like upstream `mlir-opt`. This page documents the orderings that are known to be correct and that the toolchain is built around.

![Default pass pipeline](../assets/figures/pass-pipeline-default.svg)

## End-to-end recommended order

Given an HIR module `program.mlir`, the canonical lowering to LIR is:

```sh
qoala-opt program.mlir \
  --qnet-peephole-optimizations \
  --qnet-dead-code-elimination \
  --lower-qoala-hir-to-mir \
  --lower-qoala-mir-to-lir
```

This expands to the following logical stages (the last two are bundled by the wrapper passes):

1. **HIR optimizations** (optional but recommended)
   - `qnet-check-linear` — verifier; run before any rewrite that assumes linearity.
   - `qnet-peephole-optimizations` — Hermitian cancellation + rotation folding by default. Sub-options are documented in [Passes / HIR](../passes/hir.md).
   - `qnet-dead-code-elimination` — toggle `with-classical-awareness=true` to scope DCE to measurements whose outcome is observed.

2. **HIR → MIR**
   - `lower-qoala-hir-to-mir` — full QNet → QMem lowering; introduces explicit `i32` qubit pointers and side-effecting ops.

3. **MIR helpers** (run by `lower-qoala-mir-to-lir` internally; can also be invoked individually)
   - `lower-float-rotations` — convert f32-angle rotations to integer-pair `(n, e)` form. Required because NetQASM only accepts the integer encoding.
   - `unfold-comm-ops` — split multi-value classical comm ops into single-value variants. Disable with `--lower-qoala-mir-to-lir=disable-unfold-comm-ops=true`.
   - `fold-constants` — generic constant folding inside nested ops.
   - `functionize` — extract groups of contiguous quantum ops into NetQASM routines. Tuned by `max-ops-per-group`.
   - `insert-convert-float-angle` — inject the `__qoala_convert_float_angle` runtime declaration whenever runtime angle discretization is needed.

4. **MIR → LIR**
   - `lower-qoala-mir-to-lir` (recommended) — wraps `functionize` and `lower-qmem-to-lower-dialects`. Pass options:
     - `use-simple-functionize=bool` (default `false`) — fall back to the per-op simple functionize. **PoC only**, not for production.
     - `use-sccp=bool` (default `false`) — apply SCCP to propagate constants into nested blocks of `main_func` before the lowering.
     - `max-ops-per-group=uint32` (default `0`) — cap the size of per-qubit op groups when functionizing.
     - `disable-unfold-comm-ops=bool` (default `false`) — skip the multi-value → single-value unfolding.
     - `use-online-scheduler=bool` (default `false`) — add conservative floater-to-branch dependencies on EPR (QC) blocks. The default (`false`) lets the static scheduler pipeline EPR generation with Pauli corrections.

5. **LIR finalization** (run separately as needed)
   - `qoalahost-add-block-precedences` — materialize predecessor edges in `qoalahost.blk_meta`. Honors `use-online-scheduler`.
   - `qoalahost-reorder-blocks` — MILP-driven qubit-lifetime reduction. Set `with-deadlines=true` to also compute and record deadlines in the `blk_meta` dictionary.
   - `qoalahost-show-analysis-*` — print analyses to stdout (see [Passes / LIR](../passes/lir.md)).

6. **Translation**
   ```sh
   qoala-translate --mlir-to-iqoala lowered.mlir > program.iqoala
   ```

## Mandatory vs. optional

Passes that are **mandatory** for the resulting LIR to be valid:

- `lower-qoala-hir-to-mir`
- `lower-float-rotations` (NetQASM cannot consume f32 angles)
- `lower-qoala-mir-to-lir` (or its constituent `functionize` + `lower-qmem-to-lower-dialects`)
- `qoalahost-add-block-precedences` (the runtime expects `blk_meta` to be populated)

Passes that are **optimizations** — safe to skip if you only care about correctness:

- `qnet-peephole-optimizations`, `qnet-dead-code-elimination`
- `qoalahost-reorder-blocks`
- `fold-constants`
- All `*-show-analysis-*` passes (these are observers, they do not transform IR).

## Cost-model flags

The `--qoala-opt-*` cost-model flags (e.g. `--qoala-opt-single-gate-duration`, `--qoala-opt-link-duration`, `--qoala-opt-program-horizon`) feed:

- the analysis-print passes (`gate-count`, `qubit-life`, `qmem-eff`, `esp`),
- the MILP solver inside `qoalahost-reorder-blocks`.

They have no effect on lowering correctness — only on the numbers reported by analyses and on the MILP objective. Full table in [tools/qoala-opt](../tools/qoala-opt.md).

## Standard MLIR debugging knobs

`qoala-opt` exposes the standard `mlir-opt` debugging flags:

- `--debug`, `--debug-only=<filter>` — enable `LLVM_DEBUG` prints, optionally filtered by `DEBUG_TYPE`.
- `--mlir-print-op-generic`, `--mlir-print-assume-verified` — useful when debugging custom verifiers.
- `--mlir-disable-threading` — serialize pass execution; necessary to keep debug output ordered.
- `--mlir-print-value-users` — annotate the printed IR with value-user comments.
- `--print-ir-{after,before}=<pass>`, `--print-{after,before}-all` — dump IR around specific passes.
- `--view-op-graph` — print a Graphviz representation of the op tree.

These pass through unchanged from `MlirOptMain`.
