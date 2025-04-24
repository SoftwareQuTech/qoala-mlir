set(LLVM_LINK_COMPONENTS
    Core
    Support
    nativecodegen
    native
)

# Since this tool is not a mandatory part of the compiler
# we exclude it from the "all" build target
add_mlir_tool(interpreter EXCLUDE_FROM_ALL
    interpreter.cpp
)
llvm_update_compile_flags(interpreter)
target_link_libraries(interpreter PRIVATE
    MLIRAnalysis
    MLIRExecutionEngine
    MLIRIR
    MLIRJitRunner
    MLIRLLVMDialect
    MLIRLLVMToLLVMIRTranslation
    MLIRToLLVMIRTranslationRegistration
    MLIRParser
    MLIRTargetLLVMIRExport
    MLIRSupport
)
mlir_check_all_link_libraries(interpreter)
