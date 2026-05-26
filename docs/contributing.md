# Contributing

Thanks for considering a contribution. This page covers the practical bits: how to set up your environment, where new code typically goes, and how this repo interacts with [euqalyptus](https://softwarequtech.github.io/euqalyptus/).

## Set up your dev environment

Follow [Developer's guide / Building from source](developer-guide/build-from-source.md) end-to-end. The build chain is heavy (LLVM plus SCIP plus the qoala-mlir tree itself), but only the qoala-mlir incremental rebuild is fast once everything is wired up. For day-to-day development you can expect an MLIR/LLVM build under `llvm/build/` (or `/opt/mlir/` after install), a SCIP install (the codebase is validated against 9.2.2), a qoala-mlir build under `build/`, and a Python venv with `llvm/mlir/python/requirements.txt` installed. Iterating on a single dialect or pass is fast: `cmake --build build` rebuilds only what changed.

## Where to put new code

| Adding… | Goes in |
| --- | --- |
| A new pass on an existing dialect | `include/Dialect/<X>/Passes.td` (declaration) + `lib/Analysis/<X>/<MyPass>.cpp` (implementation). Register it in `tools/qoala-opt/qoala-opt.cpp` if it's a new dialect. |
| A new conversion between dialects | `include/Conversion/<From>To<To>/<…>.td` + `lib/Conversion/<From>To<To>/<…>.cpp`. Register the produced `register*Passes()` function in `tools/qoala-opt/qoala-opt.cpp`. |
| A new op on an existing dialect | `include/Dialect/<X>/<X>Ops.td` and (if needed) C++ verifier in `lib/Dialect/<X>/<X>.cpp`. |
| A new dialect | Mirror the existing layout under `include/Dialect/`, `lib/Dialect/`, and `lib/Analysis/`. Register it in `tools/qoala-opt/qoala-opt.cpp`. |
| A new translation target (e.g. a different .iqoala variant) | `include/Target/<Name>/`, `lib/Target/<Name>/`, and a `register*Translations()` call in `tools/qoala-translate/qoala-translate.cpp`. |
| A change to the `qnet` Python bindings | `lib/Python/mlir_qnet/dialects/qnet.py` (Python-side conveniences) or `lib/Python/QNetExtension.cpp` (custom types). The TableGen-driven op bindings are auto-generated from `Dialect/QNet/QNet.td`. |

For the mechanical details of declaring passes, ops, and dialects, see the [Developer's guide](developer-guide/working-with-mlir.md).

## Tests

The lit test suite lives in `test/`, organized into `test/IR/{HIR,MIR,LIR}/` for the canonical-form tests at each IR, `test/Passes/` for per-pass behavioral tests, `test/Conversion/{HIRtoMIR,MIRtoLIR}/` for the lowerings, `test/Translation/` for the `qoala-translate` output, and `test/Angles/` for the angle-discretization tests. Run the suite from the repo root with `./llvm/build/bin/llvm-lit test`. `qoala-opt` must be on `PATH` for the tests to find it. When you add a pass or op, add a corresponding `.mlir` test (input → expected output via `// RUN:` and `// CHECK:` lines); existing tests are good templates.

## Code style

C++ is formatted with `clang-format-18` — other versions reformat the codebase in incompatible ways. The `scripts/check-clang-format.sh` script runs the check, and it is expected to be clean before merge. TableGen `.td` files follow MLIR upstream conventions: ops grouped by purpose, `let summary` always set, `let description` only when there is something non-obvious to say. C++ namespaces match the MLIR convention: `qoala::dialects::<dialect>` for dialect code, `qoala::analysis` for passes (this is the namespace the `GEN_PASS_REGISTRATION` macro targets), `qoala::conversion` for conversions, `qoala::translate` for translations.

## How qoala-mlir relates to euqalyptus

[euqalyptus](https://softwarequtech.github.io/euqalyptus/) is the user-facing Python frontend. Its only direct dependency on this repo is the `qnet` Python bindings package described in [Python bindings](bindings.md). The contract between the two is simple: euqalyptus compiles user Python into a `qnet`-dialect MLIR module (Qoala HIR); that module is fed into `qoala-opt`, which lowers it through MIR to LIR; and `qoala-translate --mlir-to-iqoala` produces the textual `.iqoala` executable.

A change to a `qnet` op or to the `qnet` Python builders has a direct impact on euqalyptus's emitted IR, while additions to euqalyptus's SDK do not affect this repo unless they introduce new HIR ops. When making changes that span both repos, the cleanest order is to land the new ops or passes in qoala-mlir first (the `qnet` Python bindings get updated automatically when you rebuild), then update the euqalyptus AST builders to use the new ops, and finally add tests on both sides — `test/IR/HIR/*.mlir` here, `tests/bindings/*.py` over there.

## Reporting issues / proposing changes

Open an issue in the repository's tracker. The most useful issues include the minimal `.mlir` (for backend bugs) or Python snippet (for frontend bugs) that reproduces the problem, the exact pass invocation if applicable (`qoala-opt input.mlir --pass-1 --pass-2=opt=val …`), and the output of `qoala-opt --version` (or the commit hash you built from). For new features, a short design note in the issue helps reviewers a lot — especially when adding a new dialect, a new pass with non-trivial cost-model interactions, or a new translation target.
