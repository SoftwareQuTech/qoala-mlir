add_subdirectory(Analysis)

add_mlir_dialect_library(MLIRHir
    HirDialect.cpp
    Hir.cpp
    HirTypes.cpp
    HirPasses.cpp

    ADDITIONAL_HEADER_DIRS
    ${PROJECT_SOURCE_DIR}/include/Dialect/hir

    DEPENDS
    MLIRHirIncGen
    MLIRHirPassIncGen

    LINK_LIBS PUBLIC
    MLIRIR
    MLIRInferTypeOpInterface
    MLIRFuncDialect

    LINK_LIBS PRIVATE
    MLIRAnalysis
    MLIRParser
    MLIRSideEffectInterfaces
    MLIRTransforms
)
