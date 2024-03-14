add_mlir_conversion_library(MLIRHirToLirPass
        HirToLir.cpp

        ADDITIONAL_HEADER_DIRS
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/hir
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/lowering

        DEPENDS
        MLIRHirIncGen
        MLIRLirIncGen
        MLIRHirToLirPassIncGen
)
