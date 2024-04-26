add_mlir_dialect_library(MLIRQoalaHost
    QoalaHostDialect.cpp
    QoalaHost.cpp

    DEPENDS
    MLIRQoalaHostIncGen
    QoalaAnalysisHelpers

    LINK_LIBS PUBLIC
    QoalaAnalysisHelpers
    MLIRIR

    LINK_LIBS PRIVATE
    QoalaAnalysisHelpers
    MLIRAnalysis
    MLIRIR
    MLIRParser
    MLIRSideEffectInterfaces
    MLIRTransforms
)
