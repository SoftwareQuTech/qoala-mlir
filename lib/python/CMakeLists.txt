include(AddMLIRPython)

declare_mlir_python_sources(QoalaHirPythonSources)
declare_mlir_python_sources(QoalaHirPythonSources.Dialects
        ADD_TO_PARENT QoalaHirPythonSources)

declare_mlir_dialect_python_bindings(
        ADD_TO_PARENT QoalaHirPythonSources.Dialects
        ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/mlir_qoalahir"
        TD_FILE dialects/HirOps.td
        SOURCES
            dialects/hir.py
        DIALECT_NAME hir)

add_mlir_python_modules(QoalaHirPythonModules
        ROOT_PREFIX "${MLIR_BINARY_DIR}/python_packages/qoala_hir_bindings/mlir"
        INSTALL_PREFIX "python_packages/qoala_hir_bindings/mlir"
        DECLARED_SOURCES
            QoalaHirPythonSources
            #MLIRPythonExtension.RegisterEverything
            #MLIRPythonSources
        #COMMON_CAPI_LINK_LIBS
        #MLIRPythonCAPI
)
