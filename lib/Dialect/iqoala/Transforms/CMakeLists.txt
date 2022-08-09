add_mlir_dialect_library(MLIRiQoalaTransforms
    iQoalaTransforms.cpp

    DEPENDS
    MLIRiQoalaIncGen
    MLIRiQoalaTransformsIncGen
    )

target_link_libraries(MLIRiQoalaTransforms PUBLIC MLIRIR)