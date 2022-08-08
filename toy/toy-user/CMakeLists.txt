get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
get_property(conversion_libs GLOBAL PROPERTY MLIR_CONVERSION_LIBS)
message("${dialect_libs}")
set(LIBS
    ${dialect_libs}
    ${conversion_libs}
    MLIROptLib
    )
add_llvm_executable(toy-user toy-user.cpp)

llvm_update_compile_flags(toy-user)
target_link_libraries(toy-user PRIVATE ${LIBS})
