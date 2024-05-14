add_mlir_conversion_library(MLIRQMemToQoalaHostPass
        QoalaMIRToQoalaLIR.cpp
        QoalaMIRToQoalaLIRPatterns.cpp
        QMemToQoalaHost.cpp
        QMemToNetQASM.cpp
        QMemToQRemote.cpp

        ADDITIONAL_HEADER_DIRS
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/QMem
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/NetQASM
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Conversion/QoalaMIRToQoalaLIR
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Conversion/helpers

        LINK_LIBS
        QoalaConversionHelpers

        DEPENDS
        QoalaConversionHelpers
        MLIRQMemIncGen
        MLIRQoalaHostIncGen
        MLIRNetQASMIncGen
        MLIRQoalaMIRToQoalaLIRPassIncGen
)
