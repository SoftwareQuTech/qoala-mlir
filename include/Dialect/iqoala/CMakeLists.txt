# The below command does the following:
#   Set iQoala.td as LLVM_TARGET_DEFINITIONS
#   Add mlir_tablegen commands for generating .h.inc and .cpp.inc files for
#       ops: iQoala.cpp.inc and iQoala.h.inc
#       types: iQoalaTypes.cpp.inc and iQoalaTypes.h.inc
#       dialect: iQoalaDialect.cpp.inc and iQoalaDialect.h.inc
#   Create a cmake target called MLIRiQoalaIncGen
add_mlir_dialect(iQoala iqoala)

add_subdirectory(Transforms)