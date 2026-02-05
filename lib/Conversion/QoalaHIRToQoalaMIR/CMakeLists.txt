add_mlir_conversion_library(MLIRQoalaHIRtoQoalaMIRPass
        QoalaHIRToQoalaMIR.cpp
        HIRToMIRHelpers.cpp
        QoalaHIRToQoalaMIRPatterns.cpp

        ADDITIONAL_HEADER_DIRS
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/QNet
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Conversion/QoalaHIRToQoalaMIR
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Conversion/helpers

        LINK_LIBS
        QoalaConversionHelpers

        DEPENDS
        QoalaConversionHelpers
        MLIRQNetIncGen
        MLIRQMemIncGen
        MLIRQoalaHIRToQoalaMIRPassIncGen
)
