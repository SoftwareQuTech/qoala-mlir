# An out-of-tree MLIR dialect

This is an example of an out-of-tree [MLIR](https://mlir.llvm.org/) dialect along with a standalone `opt`-like tool to operate on that dialect.

## Building

This setup assumes that you have built LLVM and MLIR in `$BUILD_DIR` and installed them to `$PREFIX`. To build and launch the tests, run
```sh
mkdir build && cd build
cmake -G Ninja .. -DMLIR_DIR=$PREFIX/lib/cmake/mlir -DLLVM_EXTERNAL_LIT=$BUILD_DIR/bin/llvm-lit
cmake --build . --target check-standalone
```
To build the documentation from the TableGen description of the dialect operations, run
```sh
cmake --build . --target mlir-doc
```
**Note**: Make sure to pass `-DLLVM_INSTALL_UTILS=ON` when building LLVM with CMake in order to install `FileCheck` to the chosen installation prefix.


$PREFIX (install location) is by default `/usr/local`.
$BUILD_DIR (build location) is by default the `build` directory inside the `llvm-project` git repo.
Cannot use "~" in the cmake command, have to use `/home/<user>` !


## Print graph

From within the `graphs` directory:

```
../build/bin/hir-opt ../programs/bqc_server.mlir --view-op-graph 2>&1 >/dev/null | tee bqc_server_hir.gv
``` 

View graph with `xdot bqc_server_hir.gv`.


## Build LLVM
From within the `./llvm` directory (top level of submodule):

Create `build` directory:
```
mkdir build && cd build
```

Configure:
```
cmake -G Ninja ../llvm -DLLVM_ENABLE_PROJECTS=mlir -DLLVM_BUILD_EXAMPLES=ON -DLLVM_TARGETS_TO_BUILD="X86" -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_ASSERTIONS=ON
```

Build (should produce files in `./llvm/build`)
```
cmake --build .
```

Install (should produce files in `/usr/local/lib`, `/usr/local/include` and `/usr/local/bin`):
```
sudo cmake --build . --target install
```


# Targets

`add_mlir_dialect(Toy toy)` adds the target `MLIRToyIncGen`
This target does the following:
- `mlir-tblgen -gen-op-decls`, producing `Toy.h.inc`
- `mlir-tblgen -gen-op-defs`, producing `Toy.cpp.inc`
- `mlir-tblgen -gen-typedef-decls`, producing `ToyTypes.h.inc`
- `mlir-tblgen -gen-typedef-defs`, producing `ToyTypes.cpp.inc`
- `mlir-tblgen -gen-dialect-decls`, producing `ToyDialect.h.inc`
- `mlir-tblgen -gen-dialect-defs`, producing `ToyDialect.cpp.inc`

Every `mlir-tblgen` run takes `Toy.td` as input. This `Toy.td` file should therefore
`include` all other relevant `.td` files (e.g. `ToyDialect.td`).


`add_mlir_dialect_library(MLIRToy)` adds the target `MLIRToy`.
It produces the library `libMLIRToy.a` inside `build/lib`.
It depends on the target 'MLIRToyIncGen`.


# Tools

`mlir-tblgen` is like a compiler that takes input source files (`.td` files) and also requires you to set include directories with `-I`. It's in `{llvm_build}/bin/` (built) or `/usr/local/bin` (installed).

E.g
```
/usr/local/bin/mlir-tblgen -gen-dialect-decls toy/include/Dialect/toy/ToyOps.td -I /usr/loca/include
```
produces the generated code for `ToyOps.h.inc` ??