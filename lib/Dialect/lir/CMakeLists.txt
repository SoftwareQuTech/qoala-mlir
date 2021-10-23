add_mlir_dialect_library(MLIRLir
    LirDialect.cpp
    LirOps.cpp

    ADDITIONAL_HEADER_DIRS
    ${PROJECT_SOURCE_DIR}/include/Dialect/lir

    DEPENDS
    MLIRLirOpsIncGen

    LINK_LIBS PUBLIC
    MLIRIR
)