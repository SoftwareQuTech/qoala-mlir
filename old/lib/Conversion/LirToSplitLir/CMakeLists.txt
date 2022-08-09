add_mlir_dialect_library(LirToSplitLir
  ConvertLirToSplitLir.cpp
  SplitFuncPass.cpp

  ADDITIONAL_HEADER_DIRS
  ${PROJECT_SOURCE_DIR}/include/Conversion/LirToSplitLir

  DEPENDS
  MLIRLirToSplitLirPassIncGen
)

target_link_libraries(LirToSplitLir PUBLIC MLIRIR)
