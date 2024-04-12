add_mlir_conversion_library(MLIRQNetToQMemPass
        QNetToQMem.cpp
        QNetToQMemPatterns.cpp

        ADDITIONAL_HEADER_DIRS
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/QNet
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Conversion/QNetToQMem
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Conversion/helpers

        LINK_LIBS
        QoalaConversionHelpers

        DEPENDS
        QoalaConversionHelpers
        MLIRQNetIncGen
        MLIRQMemIncGen
        MLIRQNetToQMemPassIncGen
)
