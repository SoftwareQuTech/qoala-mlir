add_mlir_dialect_library(MLIRQoalaHir
        QoalaHirTypes.cpp
        QoalaHirDialect.cpp
        QoalaHirOps.cpp
        QoalaHirPasses.cpp

        ADDITIONAL_HEADER_DIRS
        ${PROJECT_SOURCE_DIR}/include/Dialect/hir

        DEPENDS
        MLIRHirIncGen
        MLIRHirIncGen

        LINK_LIBS PUBLIC
        MLIRIR
        MLIRInferTypeOpInterface
        MLIRFuncDialect
)
