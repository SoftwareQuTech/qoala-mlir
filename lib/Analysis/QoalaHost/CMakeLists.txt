add_mlir_dialect_library(MLIRQoalaHostAnalysis
    IsolateOperations.cpp
    Precedences.cpp
    AddPrecedences.cpp
    BlockReordering.cpp
    MILPReordering.cpp
    QMemoryEfficiency.cpp
    Show.cpp

    ADDITIONAL_HEADER_DIRS
    ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/QoalaHost

    DEPENDS
    MLIRQoalaHostPassIncGen
)

target_include_directories(MLIRQoalaHostAnalysis
  PRIVATE
    ${SCIP_INCLUDE_DIR}
)

target_compile_options(obj.MLIRQoalaHostAnalysis PRIVATE
  -Wno-macro-redefined
)
