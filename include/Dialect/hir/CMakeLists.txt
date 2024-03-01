add_mlir_dialect(Hir hir)

set(LLVM_TARGET_DEFINITIONS Passes.td)
mlir_tablegen(Passes.h.inc -gen-pass-decls -name Hir)
mlir_tablegen(Passes.capi.h.inc -gen-pass-capi-header --prefix Hir)
mlir_tablegen(Passes.capi.cpp.inc -gen-pass-capi-impl --prefix Hir)
add_public_tablegen_target(MLIRHirPassIncGen)