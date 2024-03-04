# Qoala MLIR

## LLVM git submodule
*IMPORTANT:* This repository contains a link to the LLVM repository as a git submodule.
Right after checking out this repository, you need to initialize and checkout the LLVM
submodule by running:
```sh
git submodule update --init
```

## Required packages
Before going into the compilation, we need to install a few build tools. In Debian-like distros:
```shell
sudo apt-get install build-essential cmake ninja-build python3-full python3-dev
```

## Recommended packages
By default, you can use `gcc` as the compiler for LLVM and this repo, but `clang` works much faster
and it's better for optimization.

To install clang, you can follow the [official LLVM documentation (for Debian-based distros)](https://apt.llvm.org/).
To install clang/LLVM 17 run:
```shell
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 17 all
```

## Prepare the python environment
Create a python virtual environment:
```shell
cd /path/to/your/venvs
python3 -m venv llvm-venv # "llvm-venv" is the name of out python virtual environment
```

Activate the python virtual environment and install the [requirements for the MLIR python bindings](https://mlir.llvm.org/docs/Bindings/Python/#pre-requisites)
```shell
source /path/to/your/venvs/llvm-venv/bin/activate
(llvm-venv)$ cd llvm # The root folder of the llvm project
(llvm-venv)$ pip install -r mlir/python/requirements.txt
```

## Build LLVM
_With the python virtual environment activated_, navigate to the `./llvm` directory (top level of submodule):

Create `build` directory:
```shell
mkdir build && cd build
```

Configure (Optionally, set the clang compiler here).
**IMPORTANT**: It is highly recommended to use the `-DCMAKE_INSTALL_PREFIX` option to configure a different
installation prefix of the LLVM/MLIR files. In the example below, we use the prefix `/opt/mlir`, so LLVM/MLIR will be
installed in the `/opt/mlir/usr/local` folder. This is needed to _avoid leaving another clang/LLVM installation in an
inconsistent state_, so we can use clang as the compiler, and the required version of the MLIR for this repository.
Please also note that the installation prefix needs to be configured **before compiling** LLVM/MLIR; it **cannot**
be specified when running the `install` target later.
```shell
cmake -G Ninja ../llvm \
      -DCMAKE_C_COMPILER=clang-17 \
      -DCMAKE_CXX_COMPILER=clang++-17 \
      -DCMAKE_LINKER=ld.ldd-17 \
      -DCMAKE_INSTALL_PREFIX=/opt/mlir \
      -DLLVM_ENABLE_PROJECTS=mlir \
      -DLLVM_BUILD_EXAMPLES=ON \
      -DLLVM_TARGETS_TO_BUILD="X86" \
      -DCMAKE_BUILD_TYPE=Release \
      -DLLVM_ENABLE_ASSERTIONS=ON \
      -DLLVM_INSTALL_UTILS=ON \
      -DMLIR_ENABLE_BINDINGS_PYTHON=ON \
      -DPython3_EXECUTABLE=/path/to/your/venvs/llvm-venv/bin/python3
```
*Warning*: We need to specify the python executable _from the virtual environment_. Otherwise, the MLIR python
requirements will not be available when needed.

Build llvm according to the configuration (should produce the compiled files in `./llvm/build`)
```shell
cmake --build .
```

Install (should install all the binaries in `/usr/local/lib`, `/usr/local/include` and `/usr/local/bin`):
```shell
sudo cmake --build . --target install
```

## Build this repo

This setup assumes that you have built LLVM and MLIR in `./llvm/build` and installed them to `/opt/mlir/usr/local` (as
configured in the LLVM/MLIR compilation line shown above). Before running this command, pleas make sure that you have
activated the python virtual environment created for LLVM.
To build everything, run
```sh
(llvm-venv)$ mkdir build && cd build
(llvm-venv)$ cmake -G Ninja .. -DMLIR_DIR=/opt/mlir/usr/local/lib/cmake/mlir
(llvm-venv)$ cmake --build . 
```

## Build a specific dialect
All from within `build/`:

Try to run tablegen for the Hir dialect (uses stuff in `include/Dialect/hir`):
```sh
cmake --build . --target MLIRHirIncGen
```

Try to build Netqasm dialect library (uses stuff in `lib/Dialect/hir`, relies on `MLIRHirIncGen`):
```sh
cmake --build . --target MLIRHir
```

Try to build Netqasm dialect optimizer tool (uses stuff in `tools/hir`, relies on `MLIRHir`):
```sh
cmake --build . --target hir-opt
```

## Run tests
Note: `hir-opt` needs to be on your PATH.
(TODO: figure out how to point `llvm-lit` to `build/bin/hir-opt` instead.)

```sh
./llvm/build/bin/llvm-lit test
```


## Print graph

From within the `graphs` directory:

```
../build/bin/hir-opt ../programs/bqc_server.mlir --view-op-graph 2>&1 >/dev/null | tee bqc_server_hir.gv
``` 

View graph with `xdot bqc_server_hir.gv`.



