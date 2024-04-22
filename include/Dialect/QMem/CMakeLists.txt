add_mlir_dialect(QMem qmem)

set(LLVM_TARGET_DEFINITIONS QMemInterface.td)
mlir_tablegen(QMemInterfaces.h.inc -gen-op-interface-decls -name QMem)
mlir_tablegen(QMemInterfaces.cpp.inc -gen-op-interface-defs --prefix QMem)

set(LLVM_TARGET_DEFINITIONS Passes.td)
mlir_tablegen(Passes.h.inc -gen-pass-decls -name QMem)
mlir_tablegen(Passes.capi.h.inc -gen-pass-capi-header --prefix QMem)
mlir_tablegen(Passes.capi.cpp.inc -gen-pass-capi-impl --prefix QMem)
add_public_tablegen_target(MLIRQMemPassIncGen)