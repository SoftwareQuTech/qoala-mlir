# Add a cmake target for generating transformation code.
set(LLVM_TARGET_DEFINITIONS QoalaHirTransforms.td)
mlir_tablegen(QoalaHirTransforms.h.inc -gen-rewriters)
add_public_tablegen_target(MLIRQoalaHirTransformsIncGen)
