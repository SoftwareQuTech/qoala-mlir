add_mlir_dialect_library(MLIRQoalaLirM
    QoalaLirMDialect.cpp
    QoalaLirM.cpp

    DEPENDS
    MLIRQoalaLirMIncGen

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