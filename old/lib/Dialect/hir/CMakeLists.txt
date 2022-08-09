add_mlir_dialect_library(MLIRHir
    HirDialect.cpp
    HirOps.cpp

    ADDITIONAL_HEADER_DIRS
    ${PROJECT_SOURCE_DIR}/include/Dialect/hir

    DEPENDS
    MLIRHirOpsIncGen

    LINK_LIBS PUBLIC
    MLIRIR
)

add_subdirectory(Transforms)