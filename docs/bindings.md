# Python bindings

The qoala-mlir build produces an MLIR Python bindings package named `qnet`. This is the package that the [euqalyptus](<EUQALYPTUS_DOCS_URL>) frontend imports to construct Qoala HIR programmatically.

![Python bindings architecture](assets/figures/python-bindings-arch.svg)

## What's in the package

After a successful build (`build/python_packages/qnet_bindings/qnet/`), the importable surface is:

- `qnet.ir` — re-exported from MLIR's core Python API. The names you'll typically use:
  - `Module`, `Context`, `Location`, `InsertionPoint`, `Operation`, `Block`, `Region`.
- `qnet.dialects.qnet` — Python builders for every operation in the [QNet](reference/qnet.md) dialect, generated from `lib/Python/mlir_qnet/dialects/QNetOps.td` plus `qnet.py`.
- `qnet._mlir_libs` — the bundled C/C++ runtime libraries. Not imported directly.

The release wheel attached to the [GitHub releases page](<QOALA_MLIR_RELEASES_URL>) ships the same package contents along with the `qoala-opt` and `qoala-translate` binaries under `<wheel>.data/scripts/`. To build the wheel yourself, see [Developer's guide / Building from source / Build a wheel](developer-guide/build-from-source.md#9-build-a-wheel).

## How euqalyptus consumes it

[euqalyptus](<EUQALYPTUS_DOCS_URL>)'s `QoalaModule.generate_qoala_hir()` walks its internal AST and, for each node, calls into `qnet.dialects.qnet` to produce the equivalent MLIR op. The output is a `qnet.ir.Module` whose `.operation.get_asm()` is the textual HIR fed into `qoala-opt`.

If you are building tools on top of qoala-mlir directly (without euqalyptus), the same idiom applies:

```python
from qnet.dialects import qnet
from qnet.ir import Context, Location, Module, InsertionPoint

with Context() as ctx, Location.unknown() as loc:
    module = Module.create()
    with InsertionPoint(module.body):
        # Module-scope ops here, e.g. qnet.remote.
        ...
    print(str(module))
```

## Build artifacts and how it's wired

The relevant CMake configuration is in `lib/Python/CMakeLists.txt`:

- `mlir_configure_python_dev_packages()` resolves the Python dev environment from MLIR.
- `add_compile_definitions("MLIR_PYTHON_PACKAGE_PREFIX=qnet.")` sets the target package prefix to `qnet.`.
- `declare_mlir_python_sources(QNetPythonSources)` declares the Python source roots.
- `declare_mlir_dialect_python_bindings(... TD_FILE dialects/QNetOps.td DIALECT_NAME qnet)` invokes `mlir-tblgen -gen-python-op-bindings` against `lib/Python/mlir_qnet/dialects/QNetOps.td` (which itself just `include`s the canonical `Dialect/QNet/QNet.td`).
- `declare_mlir_python_extension(QNetPythonSources.Extension MODULE_NAME _qnetTypes ... SOURCES QNetExtension.cpp ...)` builds the C++ extension that registers the `!qnet.qubit` type with the Python bindings (the dialect-binding generator only emits ops, so types are registered manually).
- `add_mlir_python_common_capi_library(QNetPythonCAPI ...)` produces the bundled native library that every binding-using Python process loads.
- `add_mlir_python_modules(QNetPythonModules ...)` wires it all into the final installable Python package.

## Using the bindings without installing the wheel

If you don't want to build a wheel, point `PYTHONPATH` at the build tree. Either edit your venv's `activate` script:

```sh
export PYTHONPATH=$PYTHONPATH:/path/to/this/repo/build/python_packages/qnet_bindings
```

…or set it on the command line:

```sh
PYTHONPATH=$PYTHONPATH:/path/to/this/repo/build/python_packages/qnet_bindings python script.py
```

You also need MLIR's runtime Python requirements installed in the same venv (`pip install -r llvm/mlir/python/requirements.txt`).

## Other dialects

Only the `qnet` dialect has a Python builder layer in this repo, because that's the only one euqalyptus emits. The MIR/LIR dialects (`qmem`, `qoalahost`, `netqasm`, `qremote`) are not exposed through the Python bindings — they are produced by `qoala-opt` from HIR input.
