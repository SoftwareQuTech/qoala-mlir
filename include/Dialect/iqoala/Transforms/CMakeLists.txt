# Add a cmake target for generating transformation code.
set(LLVM_TARGET_DEFINITIONS iQoalaTransforms.td)
mlir_tablegen(iQoalaTransforms.h.inc -gen-rewriters)
add_public_tablegen_target(MLIRiQoalaTransformsIncGen)
