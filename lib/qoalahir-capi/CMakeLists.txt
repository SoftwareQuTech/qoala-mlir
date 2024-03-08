add_mlir_public_c_api_library(QoalaHirCAPI
        QoalaCAPI.cpp
        LINK_LIBS PUBLIC
        MLIRHir
)