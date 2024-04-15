add_mlir_dialect_library(MLIRQMemAnalysisLibs
        FactorizeRemoteDecls.cpp
        FunctionizeQuantumOps.cpp

        ADDITIONAL_HEADER_DIRS
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/QMem

        DEPENDS
        MLIRQMemPassIncGen
)
