include_directories(include)
add_subdirectory(include)

set(LLVM_LINK_COMPONENTS
  Support
  )

# set(LLVM_TARGET_DEFINITIONS mlir/QnirCombine.td)
# mlir_tablegen(QnirCombine.inc -gen-rewriters)
# add_public_tablegen_target(QnirCombineIncGen)

add_llvm_example(hir
  hirc.cpp
  mlir/HirDialect.cpp

  DEPENDS
  HirOpsIncGen

  )

include_directories(include/)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include/)
target_link_libraries(hir
  PRIVATE
    MLIRAnalysis
    MLIRIR
    MLIRParser
    MLIRPass
    MLIRSideEffectInterfaces
    MLIRTransforms)
