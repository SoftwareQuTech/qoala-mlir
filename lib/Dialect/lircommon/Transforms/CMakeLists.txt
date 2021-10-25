add_mlir_dialect_library(LirCommonTransforms
    LirCommonRewrites.cpp

    ADDITIONAL_HEADER_DIRS
    ${PROJECT_SOURCE_DIR}/include/Dialect/lircommon/Transforms

    DEPENDS
    MLIRLirCommonPassIncGen
    MLIRLirCommonTransformsRewritesPassIncGen
    )

target_link_libraries(LirCommonTransforms PUBLIC MLIRIR)
