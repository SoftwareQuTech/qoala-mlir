get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
get_property(conversion_libs GLOBAL PROPERTY MLIR_CONVERSION_LIBS)
set(LIBS
    ${dialect_libs}
    ${conversion_libs}
    MLIROptLib
    MLIRHir
    )
add_llvm_executable(hir-opt hir-opt.cpp)

llvm_update_compile_flags(hir-opt)
target_link_libraries(hir-opt PRIVATE ${LIBS})

mlir_check_all_link_libraries(standalone-opt)