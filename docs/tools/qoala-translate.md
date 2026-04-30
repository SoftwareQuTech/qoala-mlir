# qoala-translate

`qoala-translate` is a thin wrapper around `mlirTranslateMain`. It exists to expose a single translation:

| Translation | Input | Output |
| --- | --- | --- |
| `--mlir-to-iqoala` | An MLIR module in fully lowered LIR form (`qoalahost` + `netqasm` + `qremote`). | The textual `.iqoala` executable consumed by the Qoala runtime. |

Source: `tools/qoala-translate/qoala-translate.cpp`, with the registration in `lib/Target/iQoala/ConvertToiQoala.cpp`.

## Usage

```sh
qoala-translate --mlir-to-iqoala lowered.mlir > program.iqoala
```

The input must be a `ModuleOp`. Anything that is not a fully lowered LIR module will produce errors.

## What flags it does *not* have

`qoala-translate` registers **no `cl::opt` flags of its own**. The `qoala::options::*` symbols defined at the top of `qoala-translate.cpp` are **dummy stubs** that exist solely to satisfy the linker (the cost-model symbols are referenced by analyses linked into `qoala-translate` via shared libraries, but the tool itself does not consume them).

In particular, **none of these qoala-opt flags apply here**:

- `--qoala-opt-single-gate-duration`, `--qoala-opt-link-duration`, `--qoala-opt-program-horizon`, etc.

If you need cost-model parameters to influence the LIR before translation, pass them through `qoala-opt` first.

## Standard translation knobs

`qoala-translate` inherits the standard `mlir-translate` flags (input/output file selection, source/target language listing, etc.). Run `qoala-translate --help` to see the live list.
