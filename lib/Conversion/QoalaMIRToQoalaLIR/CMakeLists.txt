add_mlir_conversion_library(MLIRMIRToLIRPass
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
        MLIRQoalaHostAnalysis
        MLIRQMemAnalysisLibs
        MLIRMIRtoLIRHelperPasses

        DEPENDS
        QoalaConversionHelpers
        MLIRQMemIncGen
        MLIRQoalaHostIncGen
        MLIRNetQASMIncGen
        MLIRQoalaMIRToQoalaLIRPassIncGen
        MLIRMIRtoLIRHelperPasses
)

add_subdirectory(Helpers)
