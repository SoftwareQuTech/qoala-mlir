add_mlir_dialect_library(MLIRQoalaHost
    QoalaHostDialect.cpp
    QoalaHost.cpp

    DEPENDS
    MLIRQoalaHostIncGen

    LINK_LIBS PUBLIC
    MLIRIR

    LINK_LIBS PRIVATE
    MLIRAnalysis
    MLIRIR
    MLIRParser
    MLIRSideEffectInterfaces
    MLIRTransforms
)
