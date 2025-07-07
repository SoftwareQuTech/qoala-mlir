add_mlir_dialect_library(MLIRQoalaHostAnalysis
    IsolateOperations.cpp
    Precedences.cpp
    AddPrecedences.cpp
    

    ADDITIONAL_HEADER_DIRS
    ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/QoalaHost

    DEPENDS
    MLIRQoalaHostPassIncGen
)
