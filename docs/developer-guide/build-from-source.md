# Building from source

If you only want to *use* qoala-mlir, install a release wheel — see [Getting started](../getting-started.md). This page is for contributors and anyone who needs to rebuild the toolchain from a checkout (e.g. you are modifying a dialect, a pass, or the `qnet` Python bindings).

The build chain is heavy because it includes a pinned LLVM/MLIR build and a SCIP install. Plan for the first build to take an hour or more; incremental rebuilds of qoala-mlir alone are fast.

## 1. Initialize the LLVM submodule

The repository pins a specific LLVM/MLIR commit as a submodule. Since LLVM is a very dynamic project, updating this dependency has a high potential of breaking the whole build. Please stick with the provided commit reference and do not update it. After cloning:

```sh
git submodule update --init
```

This command checks out the LLVM commit recorded in the parent repository — it does **not** fetch the latest upstream LLVM. Do not run `git submodule update --remote` or otherwise advance the submodule unless you intend to migrate the toolchain to a new LLVM revision.

## 2. System packages

On Debian-like distributions:

```sh
sudo apt-get install build-essential cmake ninja-build python3-full python3-dev
```

Python **3.10, 3.11, or 3.12** is required. Python 3.13 is **not supported** because MLIR's transitive dependency on `numpy <= 1.26` does not work on 3.13.

Recommended (faster builds, better optimizations): clang/LLVM 21 (or newer) from [apt.llvm.org](https://apt.llvm.org/) and `clang-format-18` (the `scripts/check-clang-format.sh` checker is sensitive to clang-format versions).

## 3. Python virtual environment

```sh
python3 -m venv /path/to/your/venvs/llvm-venv
source /path/to/your/venvs/llvm-venv/bin/activate
```

Install MLIR's Python bindings prerequisites from inside the LLVM submodule:

```sh
pip install -r llvm/mlir/python/requirements.txt
```

## 4. Build LLVM/MLIR

From the repo root (update the `clang-21` and `ld.lld-21` commands with the corresponding clang version you installed):

```sh
cd llvm
mkdir build && cd build

cmake -G Ninja ../llvm \
      -DCMAKE_C_COMPILER=clang-21 \
      -DCMAKE_CXX_COMPILER=clang++-21 \
      -DCMAKE_LINKER=ld.lld-21 \
      -DCMAKE_INSTALL_PREFIX=/opt/mlir \
      -DLLVM_ENABLE_PROJECTS=mlir \
      -DLLVM_BUILD_EXAMPLES=ON \
      -DLLVM_TARGETS_TO_BUILD="X86" \
      -DCMAKE_BUILD_TYPE=Release \
      -DLLVM_ENABLE_ASSERTIONS=ON \
      -DLLVM_INSTALL_UTILS=ON \
      -DMLIR_ENABLE_BINDINGS_PYTHON=ON \
      -DPython3_EXECUTABLE=/path/to/your/venvs/llvm-venv/bin/python3

cmake --build .
sudo cmake --build . --target install
```

Running the `cmake --build .` command might take quite some time (several hours), since it need to build the whole LLVM/MLIR system. After this, before running the `install` command, make sure that you have write permissions on the folder that you specified in the `CMAKE_INSTALL_PREFIX` option of the first `cmake` command.

`CMAKE_INSTALL_PREFIX` matters: it must point to a writable, dedicated location (e.g. `/opt/mlir`). Setting it after the configure step does **not** work — it has to be in the initial `cmake` invocation.

## 5. Install SCIP

[SCIP](https://github.com/scipopt/scip/blob/master/INSTALL.md) is the MILP solver used by some optimization passes (notably `qoalahost-reorder-blocks`). The codebase has been proved working using **SCIP 9.2.2**; later major versions may have a different C API. For this reason, we recommend using SCIP 9.2.2.

## 6. Build qoala-mlir

From the repository root, with the venv activated (update the `clang-21` and `ld.lld-21` commands with the corresponding clang version you installed)::

```sh
mkdir build && cd build
cmake -G Ninja .. \
  -DCMAKE_C_COMPILER=clang-21 \
  -DCMAKE_CXX_COMPILER=clang++-21 \
  -DMLIR_DIR=/opt/mlir/lib/cmake/mlir \
  -DSCIP_DIR=/path/to/your/scip/lib/cmake/scip \
  -DPython3_EXECUTABLE=/path/to/your/venvs/llvm-venv/bin/python3
cmake --build .
```

This produces `build/bin/qoala-opt`, `build/bin/qoala-translate`, and the `qnet` Python bindings under `build/python_packages/qnet_bindings/`.

### Building specific dialects only

If you're iterating on one dialect, the CMake targets are granular:

```sh
# TableGen output for the QNet dialect (uses include/Dialect/QNet/)
cmake --build build --target MLIRQnetIncGen

# QNet dialect library (uses lib/Dialect/QNet/, depends on MLIRQnetIncGen)
cmake --build build --target MLIRQnet

# qoala-opt (depends on every dialect/library it links in)
cmake --build build --target qoala-opt
```

## 7. Smoke-test

```sh
export PATH="$PWD/build/bin:$PATH"
qoala-opt --help | head
qoala-translate --help | head
```

Run the lit test suite from the repository root:

```sh
./llvm/build/bin/llvm-lit test
```

(`qoala-opt` must be on `PATH` for the tests to find it.)

## 8. Use the `qnet` Python bindings without installing

Point `PYTHONPATH` at the build tree, either by editing your venv's `activate`:

```sh
export PYTHONPATH=$PYTHONPATH:/path/to/this/repo/build/python_packages/qnet_bindings
```

…or per-invocation:

```sh
PYTHONPATH=$PYTHONPATH:/path/to/this/repo/build/python_packages/qnet_bindings python script.py
```

You also need MLIR's runtime Python requirements installed in the same venv (`pip install -r llvm/mlir/python/requirements.txt`).

## 9. Build a wheel

A `pyproject.toml` at the repository root lets you build a Python wheel that bundles `qoala-opt`, `qoala-translate`, and the `qnet` Python bindings. This is the same artifact that's published as a release.

```sh
pip install -r build-requirements.txt
python -m build -w
```

Edit the `[tool.py-build-cmake.cmake]` `args` block before building to point `MLIR_DIR`, `SCIP_DIR`, and `Python3_EXECUTABLE` at the locations from steps 4–6.

The wheel ends up under `dist/`.

## Docker alternative

If you prefer a containerized build, the repository ships a `docker/llvm-scip/Dockerfile`:

```sh
LLVM_SHA=$(git rev-parse HEAD:llvm)
SCIP_VERSION=9.2.2
docker build \
  --build-arg LLVM_SHA="$LLVM_SHA" \
  --build-arg SCIP_VERSION="$SCIP_VERSION" \
  -f docker/llvm-scip/Dockerfile \
  -t qoalac/llvm-scip:$LLVM_SHA-$SCIP_VERSION .
```

## Next steps

- Adding a new pass, op, or dialect: see [Working with MLIR](working-with-mlir.md).
- Wiring it into the pipeline / writing tests: see [Contributing](../contributing.md).
