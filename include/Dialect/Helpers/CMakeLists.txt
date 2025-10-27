set(LLVM_TARGET_DEFINITIONS MIRToLIRHelperPasses.td)
mlir_tablegen(HelperPasses.h.inc -gen-pass-decls -name MIRToLIRHelpers)
add_public_tablegen_target(MLIRMIRToLIRHelperPassesIncGen)
