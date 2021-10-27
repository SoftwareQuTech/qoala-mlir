add_mlir_dialect_library(LirTransforms
    LirRewrites.cpp

    ADDITIONAL_HEADER_DIRS
    ${PROJECT_SOURCE_DIR}/include/Dialect/lir/Transforms

    DEPENDS
    MLIRLirPassIncGen
    MLIRLirTransformsRewritesPassIncGen
    )

target_link_libraries(LirTransforms PUBLIC MLIRIR)
