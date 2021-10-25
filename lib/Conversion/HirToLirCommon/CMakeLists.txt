add_mlir_dialect_library(HirToLirCommon
  ConvertHirToLirCommon.cpp

  ADDITIONAL_HEADER_DIRS
  ${PROJECT_SOURCE_DIR}/include/Conversion/HirToLirCommon

  DEPENDS
  MLIRHirToLirCommonPassIncGen
)

target_link_libraries(HirToLirCommon PUBLIC MLIRIR)
