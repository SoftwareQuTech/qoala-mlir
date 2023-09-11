get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
get_property(conversion_libs GLOBAL PROPERTY MLIR_CONVERSION_LIBS)
message("dialect libs: ${dialect_libs}")
set(LIBS
    ${dialect_libs}
    ${conversion_libs}
    MLIROptLib
    )
add_llvm_executable(lir-opt lir_opt.cpp)

llvm_update_compile_flags(lir-opt)
target_link_libraries(lir-opt PRIVATE ${LIBS})
