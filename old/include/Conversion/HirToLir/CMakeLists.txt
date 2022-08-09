set(LLVM_TARGET_DEFINITIONS Passes.td)
mlir_tablegen(Passes.h.inc -gen-pass-decls -name HirToLirConversion)
add_public_tablegen_target(MLIRHirToLirPassIncGen)
