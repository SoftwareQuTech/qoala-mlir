# Qoala MLIR

## Build LLVM
From within the `./llvm` directory (top level of submodule):

Create `build` directory:
```
mkdir build && cd build
```

Configure:
```
cmake -G Ninja ../llvm -DLLVM_ENABLE_PROJECTS=mlir -DLLVM_BUILD_EXAMPLES=ON -DLLVM_TARGETS_TO_BUILD="X86" -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_ASSERTIONS=ON -DLLVM_INSTALL_UTILS=ON
```

Build (should produce files in `./llvm/build`)
```
cmake --build .
```

Install (should produce files in `/usr/local/lib`, `/usr/local/include` and `/usr/local/bin`):
```
sudo cmake --build . --target install
```

## Build this repo

This setup assumes that you have built LLVM and MLIR in `$BUILD_DIR` and installed them to `$PREFIX`. To build everything, run
```sh
mkdir build && cd build
cmake -G Ninja .. -DMLIR_DIR=$PREFIX/lib/cmake/mlir -DLLVM_EXTERNAL_LIT=$BUILD_DIR/bin/llvm-lit
cmake --build . 
```
**Note**: Make sure to pass `-DLLVM_INSTALL_UTILS=ON` when building LLVM with CMake in order to install `FileCheck` to the chosen installation prefix.


- `$PREFIX` (install location) is by default `/usr/local`.
- `$BUILD_DIR` (build location) is by default the `build` directory inside the `llvm-project` git repo.
- Cannot use "~" in the cmake command, have to use `/home/<user>` !


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


# Tools

`mlir-tblgen` is like a compiler that takes input source files (`.td` files) and also requires you to set include directories with `-I`. It's in `{llvm_build}/bin/` (built) or `/usr/local/bin` (installed).



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
`Type` defined in `OpBase.td`, `TypeDef` defined in `AttrTypeBase.td`.
Apparently `TypeDef` should be used to create custom types.

## Useful llvm source files:
- `llvm/mlir/include/mlir/IR/OpBase.td`
- `llvm/mlir/include/mlir/IR/AttrTypeBase.td`
- `llvm/mlir/include/mlir/IR/Types.h`
