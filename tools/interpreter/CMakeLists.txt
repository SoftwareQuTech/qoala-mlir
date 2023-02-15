set(LLVM_LINK_COMPONENTS
  Core
  Support
  nativecodegen
  native
  )

add_mlir_tool(interpreter
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
