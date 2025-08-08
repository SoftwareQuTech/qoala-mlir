add_mlir_dialect_library(MLIRNetQASM
        NetQASMDialect.cpp
        NetQASM.cpp

        DEPENDS
        MLIRNetQASMIncGen
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
