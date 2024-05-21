set(LLVM_LINK_COMPONENTS
        Support
)

get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
get_property(translation_libs GLOBAL PROPERTY MLIR_TRANSLATION_LIBS)
set(LIBS
        ${dialect_libs}
        ${conversion_libs}
        MLIROptLib
        MLIRIR
        MLIRParser
        MLIRPass
        MLIRSPIRVDialect
        MLIRTranslateLib
        MLIRSupport
)
add_mlir_tool(qoala-translate qoala-translate.cpp)

llvm_update_compile_flags(qoala-translate)
target_link_libraries(qoala-translate PRIVATE ${LIBS})
mlir_check_link_libraries(qoala-translate)