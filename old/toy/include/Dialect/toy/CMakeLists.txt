# The below command does the following:
#   Set toy.td as LLVM_TARGET_DEFINITIONS
#   Add mlir_tablegen commands for generating .h.inc and .cpp.inc files for
#       ops: Toy.cpp.inc and Toy.h.inc
#       types: ToyTypes.cpp.inc and ToyTypes.h.inc
#       dialect: ToyDialect.cpp.inc and ToyDialect.h.inc
#   Create a cmake target called MLIRToyIncGen
add_mlir_dialect(Toy toy)


# Add a cmake target for generating transformation code.
set(LLVM_TARGET_DEFINITIONS ToyTransforms.td)
mlir_tablegen(ToyTransforms.h.inc -gen-rewriters)
add_public_tablegen_target(MLIRToyTransformIncGen)
