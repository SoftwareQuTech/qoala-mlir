include_directories(include)
add_subdirectory(include)

set(LLVM_LINK_COMPONENTS
  Support
  )

set(LLVM_TARGET_DEFINITIONS mlir/QnirCombine.td)
mlir_tablegen(QnirCombine.inc -gen-rewriters)
add_public_tablegen_target(QnirCombineIncGen)

add_llvm_example(qnir
  qnirc.cpp
  parser/AST.cpp
  mlir/MLIRGen.cpp
  mlir/Dialect.cpp
  mlir/QnirCombine.cpp

  DEPENDS
  QnirOpsIncGen
  QnirCombineIncGen

  )

include_directories(include/)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include/)
target_link_libraries(qnir
  PRIVATE
    MLIRAnalysis
    MLIRIR
    MLIRParser
    MLIRPass
    MLIRSideEffectInterfaces
    MLIRTransforms)
