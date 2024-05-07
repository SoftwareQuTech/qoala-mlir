add_mlir_dialect_library(MLIRQRemote
    QRemote.cpp
    QRemoteDialect.cpp

    DEPENDS
    MLIRQRemoteIncGen

    LINK_LIBS PUBLIC
    MLIRIR

    LINK_LIBS PRIVATE
    MLIRAnalysis
    MLIRIR
    MLIRParser
    MLIRSideEffectInterfaces
    MLIRTransforms
)
