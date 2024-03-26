add_mlir_dialect(Qnet qnet)

set(LLVM_TARGET_DEFINITIONS Passes.td)
mlir_tablegen(Passes.h.inc -gen-pass-decls -name Qnet)
mlir_tablegen(Passes.capi.h.inc -gen-pass-capi-header --prefix Qnet)
mlir_tablegen(Passes.capi.cpp.inc -gen-pass-capi-impl --prefix Qnet)
add_public_tablegen_target(MLIRQnetPassIncGen)