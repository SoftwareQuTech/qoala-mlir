add_mlir_dialect_library(MLIRNetQASM
    NetQASMDialect.cpp
    NetQASM.cpp

    DEPENDS
    MLIRNetQASMIncGen

    LINK_LIBS PUBLIC
    MLIRIR

    LINK_LIBS PRIVATE
    MLIRAnalysis
    MLIRIR
    MLIRParser
    MLIRSideEffectInterfaces
    MLIRTransforms
)
