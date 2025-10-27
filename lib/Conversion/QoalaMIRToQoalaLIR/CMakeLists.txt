add_mlir_conversion_library(MLIRMIRToLIRPass
        QoalaMIRToQoalaLIR.cpp
        QoalaMIRToQoalaLIRPatterns.cpp
        QMemToLowerDialects.cpp

        ADDITIONAL_HEADER_DIRS
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/QMem
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/NetQASM
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Conversion/QoalaMIRToQoalaLIR
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Conversion/helpers

        LINK_LIBS
        QoalaConversionHelpers
        MLIRQoalaHostAnalysis
        MLIRQMemAnalysisLibs

        DEPENDS
        QoalaConversionHelpers
        MLIRQMemIncGen
        MLIRQoalaHostIncGen
        MLIRNetQASMIncGen
        MLIRQoalaMIRToQoalaLIRPassIncGen
)
