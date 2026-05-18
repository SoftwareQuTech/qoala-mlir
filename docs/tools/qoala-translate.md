# qoala-translate

`qoala-translate` is the backend tool of the toolchain. It takes a fully lowered LIR module — produced by `qoala-opt` after running the HIR→MIR and MIR→LIR conversions — and emits the textual `.iqoala` executable consumed by the Qoala runtime.

| Translation | Input | Output |
| --- | --- | --- |
| `--mlir-to-iqoala` | An MLIR `ModuleOp` in fully lowered LIR form (`qoalahost` + `netqasm` + `qremote`). | The textual `.iqoala` executable. |

Source: `tools/qoala-translate/qoala-translate.cpp`, with the translation registered in `lib/Target/iQoala/ConvertToiQoala.cpp` and the translation logic in `lib/Target/iQoala/`.

## Usage

```sh
qoala-translate --mlir-to-iqoala lowered.mlir > program.iqoala
```

The input must be a `ModuleOp` in fully lowered LIR form. Anything that is not a fully lowered LIR module will produce errors.

## How the translation works

`qoala-translate` is not a pass over MLIR IR; it is a `Translation` registered with the `mlir-translate` infrastructure (compare `mlir::TranslateFromMLIRRegistration`). The entry point in `ConvertToiQoala.cpp` registers `mlir-to-iqoala` with two callbacks:

- a **dialect-registration callback** (`registerAllQoalaTranslations` + `registerAllQoalaSupportTranslations` in `QoalaTranslations.h`), which attaches a per-dialect *translation interface* to each LIR-relevant dialect: `qremote`, `netqasm`, `qoalahost`, plus the support dialects `arith`, `builtin`, `tensor`, and `cf`.
- a **function callback** that drives the actual translation: it constructs an `iQoalaContext` (per-translation state — operation-visit set, symbol resolution, etc.), calls `translateModuleToiQoala`, and finally `print()`s the resulting in-memory `iqoala::iQoalaModule` to the output stream.

`translateModuleToiQoala` (`lib/Target/iQoala/ModuleTranslation.cpp`) executes the translation in a fixed order:

1. **Translate the module-level op.** Sets up the top-level `iQoalaModule` shell.
2. **Convert routine signatures.** Walks every `netqasm.local_routine` and `netqasm.request_routine` to register their iQoala counterparts (so calls can resolve by name later).
3. **Translate remote declarations.** Every `qremote.remote @name` op is translated first, so subsequent operations can resolve their `remote` symbol attributes against the iQoala meta section.
4. **Translate `qoalahost.main_func` and recursively the routines it calls.** The host body is walked block-by-block; each `qoalahost.call` triggers translation of its callee (a `netqasm.local_routine` or `netqasm.request_routine`) if it has not yet been visited. After the walk, local routines run a fix-up pass (`resolveInternalInstrRefs`) to wire up label/branch targets internal to each routine. The `iQoalaContext` records which operations have already been visited so the same routine is not translated twice.
5. **Block-type assignment and empty-block cleanup.** Empty host blocks produced by the walk are removed, and the remaining blocks are classified as `CL`, `CC`, `QL`, or `QC` based on their content. If a block ends up with a shape that does not match one of these four types, the translation fails with a diagnostic.
6. **Translate anything left over.** Any module-level op that was not reached through the main-func walk is translated in order. Routines that reach this point are effectively dead code (never called from `main_func`); they are still emitted in the output.

Dispatch within each step is delegated to the per-dialect translation interfaces installed in the dialect registry. `ModuleTranslation::convertOperation` looks up the interface on the op's dialect and forwards to its `convertOperation(op, moduleTranslation)` method; the dialect-specific translators (under `lib/Target/iQoala/Dialect/<DialectName>/`) emit the corresponding iQoala MC instructions (see `lib/Target/iQoala/MC/`).

The final `iQoalaModule.print()` walks the in-memory model and writes the four `.iqoala` sections (meta, host, subroutines, requests) in their canonical order.

## What flags it does *not* have

`qoala-translate` registers **no `cl::opt` flags of its own**. The `qoala::options::*` symbols defined at the top of `qoala-translate.cpp` are **dummy stubs** that exist solely to satisfy the linker: the cost-model symbols are referenced by analyses linked into `qoala-translate` via shared libraries, but the tool itself does not consume them.

In particular, **none of these qoala-opt flags apply here**:

- `--qoala-opt-single-gate-duration`, `--qoala-opt-link-duration`, `--qoala-opt-program-horizon`, etc.

If you need cost-model parameters to influence the LIR before translation, pass them through `qoala-opt` first.

## Standard translation knobs

`qoala-translate` inherits the standard `mlir-translate` flags (input/output file selection, source/target language listing, etc.). Run `qoala-translate --help` to see the live list.
