add_mlir_dialect_library(MLIRHost
    HostDialect.cpp
    Host.cpp

    DEPENDS
    MLIRHostIncGen

    LINK_LIBS PUBLIC
    MLIRIR

    LINK_LIBS PRIVATE
    MLIRAnalysis
    MLIRIR
    MLIRParser
    MLIRSideEffectInterfaces
    MLIRTransforms
)
