set(LLVM_LINK_COMPONENTS
        Support
)

get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
get_property(translation_libs GLOBAL PROPERTY MLIR_TRANSLATION_LIBS)

add_mlir_tool(qoala-translate
        qoala-translate.cpp

        DEPENDS
        ${dialect_libs}
        ${translation_libs}
        MLIROptLib
        MLIRIR
        MLIRParser
        MLIRPass
        MLIRSPIRVDialect
        MLIRTranslateLib
        MLIRSupport
)

llvm_update_compile_flags(qoala-translate)
target_link_libraries(qoala-translate
        PRIVATE
        ${dialect_libs}
        ${translation_libs}
        MLIROptLib
        MLIRIR
        MLIRParser
        MLIRPass
        MLIRSPIRVDialect
        MLIRTranslateLib
        MLIRSupport
)
mlir_check_all_link_libraries(qoala-translate)
