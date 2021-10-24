add_mlir_dialect_library(MLIRCLir
    CLirDialect.cpp
    CLirOps.cpp

    ADDITIONAL_HEADER_DIRS
    ${PROJECT_SOURCE_DIR}/include/Dialect/clir

    DEPENDS
    MLIRCLirOpsIncGen

    LINK_LIBS PUBLIC
    MLIRIR
)