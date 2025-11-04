add_mlir_dialect_library(MLIRQNetAnalysis
        CheckLinear.cpp
        DeadCodeElimination.cpp
        PeepholeOptimizations.cpp

        ADDITIONAL_HEADER_DIRS
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/QNet

        DEPENDS
        MLIRQNetPassIncGen
)
