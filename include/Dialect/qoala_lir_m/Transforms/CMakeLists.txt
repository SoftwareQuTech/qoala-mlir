# Add a cmake target for generating transformation code.
set(LLVM_TARGET_DEFINITIONS QoalaLirMTransforms.td)
mlir_tablegen(QoalaLirMTransforms.h.inc -gen-rewriters)
add_public_tablegen_target(MLIRQoalaLirMTransformsIncGen)
