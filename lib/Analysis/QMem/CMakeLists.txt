add_mlir_dialect_library(MLIRQMemAnalysisLibs
        Functionize.cpp
        FunctionizeQuantumOps.cpp
        LowerF32Rotations.cpp
        SimpleOpClassifier.cpp
        PerQuibitOpClassifier.cpp

        ADDITIONAL_HEADER_DIRS
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/QMem

        DEPENDS
        MLIRSimpleCloneInterfaceIncGen
        MLIRQMemPassIncGen
)
