add_mlir_dialect_library(MLIRQMemAnalysisLibs
        Functionizing.cpp
        FloatFlattening.cpp
        FunctionizeQuantumOps.cpp

        ADDITIONAL_HEADER_DIRS
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/QMem

        DEPENDS
        MLIRSimpleCloneInterfaceIncGen
        MLIRQMemPassIncGen
)
