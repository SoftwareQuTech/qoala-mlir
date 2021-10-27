add_mlir_dialect_library(HirToLir
  ConvertHirToLir.cpp

  ADDITIONAL_HEADER_DIRS
  ${PROJECT_SOURCE_DIR}/include/Conversion/HirToLir

  DEPENDS
  MLIRHirToLirPassIncGen
)

target_link_libraries(HirToLir PUBLIC MLIRIR)
