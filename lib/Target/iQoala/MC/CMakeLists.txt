add_mlir_library(MLIRiQoalaMCLib
        iQoalaMC.cpp
        iQoalaMCSections.cpp
        iQoalaMCRoutines.cpp
        iQoalaMCBlocks.cpp
        NetQASMASM.cpp
        QoalaHostASM.cpp

        ADDITIONAL_HEADER_DIRS
        ${MLIR_MAIN_INCLUDE_DIR}/Target/iQoala
)