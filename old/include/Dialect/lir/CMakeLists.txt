add_mlir_dialect(LirOps lir)
add_mlir_doc(LirDialect LirDialect lir/ -gen-dialect-doc)
add_mlir_doc(LirTypes LirTypes lir/ -gen-op-doc)
add_mlir_doc(LirOps LirOps lir/ -gen-op-doc)

set(LLVM_TARGET_DEFINITIONS Passes.td)
mlir_tablegen(Passes.h.inc -gen-pass-decls -name Lir)
add_public_tablegen_target(MLIRLirPassIncGen)

add_subdirectory(Transforms)