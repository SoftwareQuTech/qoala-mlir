get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
get_property(conversion_libs GLOBAL PROPERTY MLIR_CONVERSION_LIBS)
set(LIBS
        ${dialect_libs}
        ${conversion_libs}
        MLIROptLib
)
add_mlir_tool(qoala-opt
        qoala-opt.cpp

        DEPENDS
        ${LIBS}
)
llvm_update_compile_flags(qoala-opt)
target_link_libraries(qoala-opt
        PRIVATE
        ${LIBS}
        libscip
)
mlir_check_all_link_libraries(qoala-opt)
