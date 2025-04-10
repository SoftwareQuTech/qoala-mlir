add_mlir_dialect(QMem qmem)

set(LLVM_TARGET_DEFINITIONS Passes.td)
mlir_tablegen(Passes.h.inc -gen-pass-decls -name QMem)
add_public_tablegen_target(MLIRQMemPassIncGen)