add_mlir_dialect(QoalaHost qoalahost)

set(LLVM_TARGET_DEFINITIONS Passes.td)
mlir_tablegen(Passes.h.inc -gen-pass-decls -name QoalaHost)
add_public_tablegen_target(MLIRQoalaHostPassIncGen)
