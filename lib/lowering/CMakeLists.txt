
message(includeA="${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/hir")
message(includeA="${MLIR_MAIN_INCLUDE_DIR}/mlir/lowering")

add_mlir_dialect_library(MLIRHirToLirPass
        HirToLir.cpp

        ADDITIONAL_HEADER_DIRS
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/hir
        ${MLIR_MAIN_INCLUDE_DIR}/mlir/lowering

        DEPENDS
        MLIRHirPassIncGen
        MLIRHirToLirPassIncGen
)
