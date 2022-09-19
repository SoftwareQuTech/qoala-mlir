get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
get_property(conversion_libs GLOBAL PROPERTY MLIR_CONVERSION_LIBS)
message("dialect libs: ${dialect_libs}")
set(LIBS
    ${dialect_libs}
    ${conversion_libs}
    MLIROptLib
    )
add_llvm_executable(qoala-lir-m-opt qoala_lir_m_opt.cpp)

llvm_update_compile_flags(qoala-lir-m-opt)
target_link_libraries(qoala-lir-m-opt PRIVATE ${LIBS})
