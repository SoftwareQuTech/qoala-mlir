add_mlir_translation_library(LirToText
    ConvertToText.cpp

    ADDITIONAL_HEADER_DIRS
    ${PROJECT_SOURCE_DIR}/include/Target/lir

    LINK_LIBS PUBLIC
    MLIRIR
    MLIRCLir
    MLIRQLir
    MLIRLir
    MLIRHir
    )

target_link_libraries(LirToText PUBLIC MLIRIR)
