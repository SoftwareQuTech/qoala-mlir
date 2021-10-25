add_mlir_dialect(LirCommonOps lircommon)
add_mlir_doc(LirCommonDialect LirCommonDialect lircommon/ -gen-dialect-doc)
add_mlir_doc(LirCommonTypes LirCommonTypes lircommon/ -gen-op-doc)
add_mlir_doc(LirCommonOps LirCommonOps lircommon/ -gen-op-doc)

set(LLVM_TARGET_DEFINITIONS Passes.td)
mlir_tablegen(Passes.h.inc -gen-pass-decls -name LirCommon)
add_public_tablegen_target(MLIRLirCommonPassIncGen)

add_subdirectory(Transforms)