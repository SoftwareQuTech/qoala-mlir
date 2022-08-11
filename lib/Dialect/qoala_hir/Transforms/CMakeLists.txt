add_mlir_dialect_library(MLIRQoalaHirTransforms
    QoalaHirTransforms.cpp

    DEPENDS
    MLIRQoalaHirIncGen
    MLIRQoalaHirTransformsIncGen
    )

target_link_libraries(MLIRQoalaHirTransforms PUBLIC MLIRIR)