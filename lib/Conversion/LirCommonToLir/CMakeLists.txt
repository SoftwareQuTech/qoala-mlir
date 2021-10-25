add_mlir_dialect_library(LirCommonToLir
  ConvertLirCommonToLir.cpp
  SplitFuncPass.cpp

  ADDITIONAL_HEADER_DIRS
  ${PROJECT_SOURCE_DIR}/include/Conversion/LirCommonToLir

  DEPENDS
  MLIRLirCommonToLirPassIncGen
)

target_link_libraries(LirCommonToLir PUBLIC MLIRIR)
