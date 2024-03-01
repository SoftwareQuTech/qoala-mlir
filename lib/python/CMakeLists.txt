include(AddMLIRPython)

declare_mlir_dialect_python_bindings(
        ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
        TD_FILE HirOps.td
        SOURCES
        hir.py
        DIALECT_NAME hir)
