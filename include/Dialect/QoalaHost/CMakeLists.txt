add_mlir_dialect(QoalaHost qoalahost)

set(LLVM_TARGET_DEFINITIONS Passes.td)
mlir_tablegen(Passes.h.inc -gen-pass-decls -name QoalaHost)
# mlir_tablegen(Passes.capi.h.inc -gen-pass-capi-header --prefix QoalaHost)
# mlir_tablegen(Passes.capi.cpp.inc -gen-pass-capi-impl --prefix QoalaHost)
add_public_tablegen_target(MLIRQoalaHostPassIncGen)
