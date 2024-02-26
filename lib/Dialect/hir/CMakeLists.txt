add_mlir_dialect_library(MLIRHir
    HirDialect.cpp
    Hir.cpp

    DEPENDS
    MLIRHirIncGen

    LINK_LIBS PUBLIC
    MLIRIR

    LINK_LIBS PRIVATE
    MLIRAnalysis
    MLIRIR
    MLIRParser
    MLIRSideEffectInterfaces
    MLIRTransforms
)
