add_mlir_conversion_library(MLIRQnetToQmemPass
        QnetToQmem.cpp
        QnetToQmemPatterns.cpp

        ADDITIONAL_HEADER_DIRS
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/Qnet
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Conversion/QnetToQmem
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Conversion/helpers

        LINK_LIBS
        QoalaConversionHelpers

        DEPENDS
        QoalaConversionHelpers
        MLIRQnetIncGen
        MLIRLirIncGen
        MLIRQnetToQmemPassIncGen
)
