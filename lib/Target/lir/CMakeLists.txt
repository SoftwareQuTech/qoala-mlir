add_mlir_translation_library(HirToLir
    ConvertToLir.cpp

    ADDITIONAL_HEADER_DIRS
    ${PROJECT_SOURCE_DIR}/include/Target/lir

    LINK_LIBS PUBLIC
    MLIRIR
    MLIRLir
    MLIRHir
    )

target_link_libraries(HirToLir PUBLIC MLIRIR)
