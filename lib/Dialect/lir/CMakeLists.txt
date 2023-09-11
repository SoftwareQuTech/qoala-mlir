add_mlir_dialect_library(MLIRLir
    LirDialect.cpp
    Lir.cpp

    DEPENDS
    MLIRLirIncGen

    LINK_LIBS PUBLIC
    MLIRIR

    LINK_LIBS PRIVATE
    MLIRAnalysis
    MLIRIR
    MLIRParser
    MLIRSideEffectInterfaces
    MLIRTransforms
)
