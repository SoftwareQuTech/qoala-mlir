add_mlir_dialect_library(MLIRQmem
    QmemDialect.cpp
    Qmem.cpp

    DEPENDS
    MLIRQmemIncGen

    LINK_LIBS PUBLIC
    MLIRIR

    LINK_LIBS PRIVATE
    MLIRAnalysis
    MLIRIR
    MLIRParser
    MLIRSideEffectInterfaces
    MLIRTransforms
)
