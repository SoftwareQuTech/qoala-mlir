add_mlir_dialect_library(MLIRQMem
        QMemDialect.cpp
        QMem.cpp

        DEPENDS
        MLIRQMemIncGen

        LINK_LIBS PUBLIC
        MLIRIR

        LINK_LIBS PRIVATE
        MLIRAnalysis
        MLIRIR
        MLIRParser
        MLIRSideEffectInterfaces
        MLIRTransforms
)
