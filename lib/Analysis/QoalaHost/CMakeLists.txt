add_mlir_dialect_library(MLIRQoalaHostAnalysis
        AddRemoteIDPlaceholders.cpp
        IsolateOperations.cpp
        Precedences.cpp
        AddPrecedences.cpp
        BlockReordering.cpp
        MILPReordering.cpp
        QubitLifetime.cpp
        QubitLifetimeAnalysis.cpp
        QMemoryEfficiency.cpp
        QuantumMemoryAnalysis.cpp
        EntanglementFirst.cpp
        EstimateSuccProb.cpp
        EstimateSuccProbAnalysis.cpp
        GateCount.cpp
        GateCountAnalysis.cpp

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
