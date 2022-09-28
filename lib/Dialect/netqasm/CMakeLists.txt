add_mlir_dialect_library(MLIRNetqasm
    NetqasmDialect.cpp
    Netqasm.cpp

    DEPENDS
    MLIRNetqasmIncGen

    LINK_LIBS PUBLIC
    MLIRIR

    LINK_LIBS PRIVATE
    MLIRAnalysis
    MLIRIR
    MLIRParser
    MLIRSideEffectInterfaces
    MLIRTransforms
)
