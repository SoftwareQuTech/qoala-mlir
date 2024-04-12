add_mlir_conversion_library(MLIRQMemToQoalaHostPass
        QMemToQoalaHost.cpp
        QMemToQoalaHostPatterns.cpp

        ADDITIONAL_HEADER_DIRS
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/QMem
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Conversion/QMemToQoalaHost
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Conversion/helpers

        LINK_LIBS
        QoalaConversionHelpers

        DEPENDS
        QoalaConversionHelpers
        MLIRQMemIncGen
        MLIRQoalaHostIncGen
        MLIRQMemToQoalaHostPassIncGen
)
