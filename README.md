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
sudo apt-get install build-essential cmake ninja python3-full python3-dev
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

## Test if everything works
All from within `build/`:

Try to run tablegen for Netqasm dialect (uses stuff in `include/Dialect/netqasm`):
```sh
cmake --build . --target MLIRNetqasmIncGen
```

Try to build Netqasm dialect library (uses stuff in `lib/Dialect/netqasm`, relies on `MLIRNetqasmIncGen`):
```sh
cmake --build . --target MLIRNetqasm
```

Try to build Netqasm dialect optimizer tool (uses stuff in `tools/netqasm`, relies on `MLIRNetqasm`):
```sh
cmake --build . --target netqasm-opt
```

Run `netqasm-opt` tool:
```sh
./bin/netqasm-opt ../programs/netqasm/netqasm1.mlir
```
Should just print the contents of `netqasm1.mlir`.

## Print graph

From within the `graphs` directory:

```
../build/bin/hir-opt ../programs/bqc_server.mlir --view-op-graph 2>&1 >/dev/null | tee bqc_server_hir.gv
``` 

View graph with `xdot bqc_server_hir.gv`.




# Targets

`add_mlir_dialect(Toy toy)` adds the target `MLIRToyIncGen`
This target does the following:
- `mlir-tblgen -gen-op-decls`, producing `Toy.h.inc` (contains the Ops)
- `mlir-tblgen -gen-op-defs`, producing `Toy.cpp.inc` (containts the Ops)
- `mlir-tblgen -gen-typedef-decls`, producing `ToyTypes.h.inc`
- `mlir-tblgen -gen-typedef-defs`, producing `ToyTypes.cpp.inc`
- `mlir-tblgen -gen-dialect-decls`, producing `ToyDialect.h.inc`
- `mlir-tblgen -gen-dialect-defs`, producing `ToyDialect.cpp.inc`

All `.inc` files appear in `build/include/Dialect/toy` (assuming the `.td` files
are in `include/Dialect/toy`).

Every `mlir-tblgen` run takes `Toy.td` as input. This `Toy.td` file should therefore
`include` all other relevant `.td` files (e.g. `ToyDialect.td`).


`add_mlir_dialect_library(MLIRToy)` adds the target `MLIRToy`.
It produces the library `libMLIRToy.a` inside `build/lib`.
It depends on the target 'MLIRToyIncGen`.


For targets in `lib/`, each `add_mlir_dialect_library` should be in its own
`CMakeLists.txt`, and therefore in its own directory.
So, a separate Transforms library would require a separate
`Transforms` directory inside `lib/Dialect/toy`, with its own `CMakeLists.txt`.


# ODS
Operations defined in ODS (`.td` file) generate the following C++ class names:
- `MyNameOp` -> `class MyNameOp`
- `Idk_MyNameOp` -> `class MyNameOp` (!!)
- `Idk_Wtf_MyNameOp` -> `class Wtf_MyNameOp`
In other words: if the name starts with `XXX_`, this part is omitted!

# Standard Ops

## Syntax
Operation with a region:
```mlir
"dialect.opname"() ( { <blocks> } ) : <function-type>
```
where the `()` indicate a list of regions, and each region is indicated by `{}`.

A region `{}` consists of blocks `^b:`.
A region `{}` may start directly with operations: they are in an implict entry block.

A block `^b:` can have arguments, written as e.g. `^b(%a: i32):`

Operation with a successor (e.g. `cf.br`):
```mlir
"dialect.opname"() [^b1] : <function-type>
```


## Module
- Dialect: `Builtin_Dialect`, name: `builtin`
- Full Op name: `builtin.module`

Traits (among others):
- `NoTerminator` (from `GraphRegionNoTerminator` list): region does not have a terminator
- `SingleBlock` (from `GraphRegionNoTerminator` list): region must have exactly 1 block
- **TODO**


## Function
- Dialect: `Func_Dialect`, name: `func`
- Full Op name: `func.func`

Traits/Interfaces (among others):
- `FunctionOpInterface`: means
  - single region
  - must have ODS `function_type` TypeAttr (used by `getFunctionType()`)
  - must have ODS `sym_name` StringAttr (used by `getSymbolAttrName()`)

## Branch
- Dialect: `ControlFlow_Dialect`, name: `cg`
- Full Op name: `cf.br`

Traits/Interfaces (among others):
- `FunctionOpInterface`: means
  - single region
  - must have ODS `function_type` TypeAttr (used by `getFunctionType()`)
  - must have ODS `sym_name` StringAttr (used by `getSymbolAttrName()`)
  - must have ODS `sym_visibility` StringAttr 

# Tools

`mlir-tblgen` is like a compiler that takes input source files (`.td` files) and also requires you to set include directories with `-I`. It's in `{llvm_build}/bin/` (built) or `/usr/local/bin` (installed).


The default `mlir-cpu-runner` in `llvm/build/bin` does not register any dialects.
`tools/interpreter` contains a copy of the source code of `mlir-cpu-runner` but does register all dialects. It can interpret ops in the LLVM Dialect (`llvm.mlir`) directly.
Other ops may or may not be valid, depending on whether there is a translation interface implemented for it.

# Misc
Be careful when `#include`ing `.h.inc` files. You may or may not want to
`#define` a certain variable just before it. For example you might need to use

```
#define GET_OP_CLASSES
#include "Dialect/toy/Toy.h.inc"
```

instead of just `#include "Dialect/toy/Toy.h.inc"`.


# Passes
Using `PassManager`s or `PassRegistration` doesn't seem to do anything.
The following does work: use 

```
mlir::registerPass([]() -> std::unique_ptr<::mlir::Pass>
                       { return createXXXPass(); });
```

Creating a `std::unique_ptr` inside the lambda doesn't seem to work; it's better to
declare a `createXXXPass` function in `MyTransforms.h` which returns a `std::unique_ptr<Pass>`.


# Tranformations
Inside a pass, one may call `applyPatternsAndFoldGreedily()` with a set of `Pattern`s.


## Type vs TypeDef
`Type` defined in `CommonTypeConstraints.td`, `TypeDef` defined in `AttrTypeBase.td`.
Apparently `TypeDef` should be used to create custom types.

## Useful llvm source files:
- `llvm/mlir/include/mlir/IR/OpBase.td`
- `llvm/mlir/include/mlir/IR/BuiltinOps.td`
- `llvm/mlir/include/mlir/IR/AttrTypeBase.td`
- `llvm/mlir/include/mlir/IR/Types.h`



## Misc
`op-result-list` cannot be surrounded by parentheses!
So you cannot write `(%0, %1) = <op> ...` but it must be `%0, %1 = <op> ...`

To print the generic (non-custom) format of MLIR operations, pass `--mlir-print-op-generic` to `mlir-opt`.