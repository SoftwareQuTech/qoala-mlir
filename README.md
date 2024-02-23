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

Activate the python virtual environemtn and install the [requirements for the MLIR python bindings](https://mlir.llvm.org/docs/Bindings/Python/#pre-requisites)
```shell
source /path/to/your/venvs/llvm-venv/bin/activate
cd llvm # The root folder of the llvm project
pip install -r mlir/python/requirements.txt
```

## Build LLVM
_With the python virtual environment activated_, navigate to the `./llvm` directory (top level of submodule):

Create `build` directory:
```shell
mkdir build && cd build
```

Configure (Optionally, set the clang compiler here:):
```shell
cmake -G Ninja ../llvm \
      -DCMAKE_C_COMPILER=clang-17 \
      -DCMAKE_CXX_COMPILER=clang++-17 \
      -DCMAKE_LINKER=ld.ldd-17 \
      -DLLVM_ENABLE_PROJECTS=mlir \
      -DLLVM_BUILD_EXAMPLES=ON \
      -DLLVM_TARGETS_TO_BUILD="X86" \
      -DCMAKE_BUILD_TYPE=Release \
      -DLLVM_ENABLE_ASSERTIONS=ON \
      -DLLVM_INSTALL_UTILS=ON \
      -DMLIR_ENABLE_BINDINGS_PYTHON=ON \
      -DPython3_EXECUTABLE=/path/to/your/venvs/llvm-venv/bin/python3
```
Please note that we need to specify the python executable _from the virtual environment_. Otherwise, the MLIR python requirements will not be available when needed.

Build llvm according to the configuration (should produce the compiled files in `./llvm/build`)
```shell
cmake --build .
```

Install (should install all the binaries in `/usr/local/lib`, `/usr/local/include` and `/usr/local/bin`):
```shell
sudo cmake --build . --target install
```

## Build this repo

This setup assumes that you have built LLVM and MLIR in `./llvm/build` and installed them to `/usr/local`. To build everything, run
```sh
mkdir build && cd build
cmake -G Ninja .. -DMLIR_DIR=/usr/local/lib/cmake/mlir
cmake --build . 
```

- Assumes that the install location is `/usr/local`.

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
```sh
./llvm/build/bin/llvm-lit test
```


## Print graph

From within the `graphs` directory:

```
../build/bin/hir-opt ../programs/bqc_server.mlir --view-op-graph 2>&1 >/dev/null | tee bqc_server_hir.gv
``` 

View graph with `xdot bqc_server_hir.gv`.



