add_mlir_dialect(QNet qnet)

set(LLVM_TARGET_DEFINITIONS Passes.td)
mlir_tablegen(Passes.h.inc -gen-pass-decls -name QNet)
mlir_tablegen(Passes.capi.h.inc -gen-pass-capi-header --prefix QNet)
mlir_tablegen(Passes.capi.cpp.inc -gen-pass-capi-impl --prefix QNet)
add_public_tablegen_target(MLIRQNetPassIncGen)