add_mlir_dialect_library(MLIRQMemAnalysisLibs
        Functionize.cpp
        FunctionizeQuantumOps.cpp
        SimpleFunctionizeQuantumOps.cpp
        FoldConstants.cpp
        LowerF32Rotations.cpp
        InsertAngleConversionDeclaration.cpp
        SimpleOpClassifier.cpp
        PerQubitOpClassifier.cpp
        ConstantRemoval.cpp

        ADDITIONAL_HEADER_DIRS
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/QMem

        DEPENDS
        MLIRQMemInterfacesIncGen
        MLIRMIRToLIRHelperPassesIncGen
)
