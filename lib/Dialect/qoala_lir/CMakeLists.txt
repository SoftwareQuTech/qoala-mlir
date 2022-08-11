add_mlir_dialect_library(MLIRQoalaLir
    QoalaLirDialect.cpp
    QoalaLir.cpp

    DEPENDS
    MLIRQoalaLirIncGen

    LINK_LIBS PUBLIC
    MLIRIR

    LINK_LIBS PRIVATE
    MLIRAnalysis
    MLIRIR
    MLIRParser
    MLIRSideEffectInterfaces
    MLIRTransforms
)

add_subdirectory(Transforms)