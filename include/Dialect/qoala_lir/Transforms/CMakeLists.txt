# Add a cmake target for generating transformation code.
set(LLVM_TARGET_DEFINITIONS QoalaLirTransforms.td)
mlir_tablegen(QoalaLirTransforms.h.inc -gen-rewriters)
add_public_tablegen_target(MLIRQoalaLirTransformsIncGen)
