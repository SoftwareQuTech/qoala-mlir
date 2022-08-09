add_mlir_dialect_library(MLIRQLir
    QLirDialect.cpp
    QLirOps.cpp

    ADDITIONAL_HEADER_DIRS
    ${PROJECT_SOURCE_DIR}/include/Dialect/qlir

    DEPENDS
    MLIRQLirOpsIncGen

    LINK_LIBS PUBLIC
    MLIRIR
)