add_mlir_dialect_library(HirTransforms
    HirRewrites.cpp

    ADDITIONAL_HEADER_DIRS
    ${PROJECT_SOURCE_DIR}/include/Dialect/hir/Transforms

    DEPENDS
    MLIRHirPassIncGen
    MLIRHirTransformsRewritesPassIncGen
    )

target_link_libraries(HirTransforms PUBLIC MLIRIR)
