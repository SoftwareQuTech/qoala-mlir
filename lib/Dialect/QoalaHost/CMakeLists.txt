add_mlir_dialect_library(MLIRQoalaHost
        QoalaHostDialect.cpp
        QoalaHost.cpp

        DEPENDS
        MLIRQoalaHostIncGen
        MLIRQoalaHostAnalysis
        QoalaDialectHelpers
        QoalaAnalysisHelpers

        LINK_LIBS PUBLIC
        QoalaAnalysisHelpers
        MLIRQoalaHostAnalysis
        MLIRIR

        LINK_LIBS PRIVATE
        QoalaAnalysisHelpers
        QoalaDialectHelpers
        MLIRQoalaHostAnalysis
        MLIRAnalysis
        MLIRIR
        MLIRParser
        MLIRSideEffectInterfaces
        MLIRTransforms
)
