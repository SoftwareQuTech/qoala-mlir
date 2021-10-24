add_mlir_dialect_library(MLIRLirCommon
    LirCommonDialect.cpp
    LirCommonOps.cpp

    ADDITIONAL_HEADER_DIRS
    ${PROJECT_SOURCE_DIR}/include/Dialect/lircommon

    DEPENDS
    MLIRLirCommonOpsIncGen

    LINK_LIBS PUBLIC
    MLIRIR
)