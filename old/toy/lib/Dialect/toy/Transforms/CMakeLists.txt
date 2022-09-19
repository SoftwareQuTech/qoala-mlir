add_mlir_dialect_library(MLIRToyTransforms
    ToyTransforms.cpp

    DEPENDS
    MLIRToyIncGen
    MLIRToyTransformIncGen
    )

target_link_libraries(MLIRToyTransforms PUBLIC MLIRIR)