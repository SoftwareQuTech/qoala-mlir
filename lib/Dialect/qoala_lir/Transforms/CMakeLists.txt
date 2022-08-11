add_mlir_dialect_library(MLIRQoalaLirTransforms
    QoalaLirTransforms.cpp

    DEPENDS
    MLIRQoalaLirIncGen
    MLIRQoalaLirTransformsIncGen
    )

target_link_libraries(MLIRQoalaLirTransforms PUBLIC MLIRIR)