add_mlir_dialect_library(MLIRQnetAnalysis
        CheckLinear.cpp

  ADDITIONAL_HEADER_DIRS
  ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/Qnet

  DEPENDS
  MLIRQnetPassIncGen
)
