add_mlir_dialect_library(MLIRQoalaLirMTransforms
    QoalaLirMTransforms.cpp

    DEPENDS
    MLIRQoalaLirMIncGen
    # MLIRQoalaLirMTransformsIncGen
    )

target_link_libraries(MLIRQoalaLirMTransforms PUBLIC MLIRIR)