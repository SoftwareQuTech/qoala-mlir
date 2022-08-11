add_mlir_dialect_library(MLIRQoalaHir
    QoalaHirDialect.cpp
    QoalaHir.cpp

    DEPENDS
    MLIRQoalaHirIncGen

    LINK_LIBS PUBLIC
    MLIRIR

    LINK_LIBS PRIVATE
    MLIRAnalysis
    MLIRIR
    MLIRParser
    MLIRSideEffectInterfaces
    MLIRTransforms
)

add_subdirectory(Transforms)