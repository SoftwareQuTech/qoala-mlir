add_mlir_dialect(QNet qnet)

set(LLVM_TARGET_DEFINITIONS Passes.td)
mlir_tablegen(Passes.h.inc -gen-pass-decls -name QNet)
add_public_tablegen_target(MLIRQNetPassIncGen)