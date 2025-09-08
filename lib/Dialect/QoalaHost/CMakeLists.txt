add_mlir_dialect_library(MLIRQoalaHost
        QoalaHostDialect.cpp
        QoalaHost.cpp

        DEPENDS
        MLIRQoalaHostIncGen
        QoalaDialectHelpers
        QoalaAnalysisHelpers

        LINK_LIBS PUBLIC
        QoalaAnalysisHelpers
        MLIRIR

        LINK_LIBS PRIVATE
        QoalaAnalysisHelpers
        QoalaDialectHelpers
        MLIRAnalysis
        MLIRIR
        MLIRParser
        MLIRSideEffectInterfaces
        MLIRTransforms
)
