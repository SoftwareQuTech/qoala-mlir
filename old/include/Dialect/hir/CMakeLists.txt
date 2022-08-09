add_mlir_dialect(HirOps hir)
add_mlir_doc(HirDialect HirDialect hir/ -gen-dialect-doc)
add_mlir_doc(HirTypes HirTypes hir/ -gen-op-doc)
add_mlir_doc(HirOps HirOps hir/ -gen-op-doc)

set(LLVM_TARGET_DEFINITIONS Passes.td)
mlir_tablegen(Passes.h.inc -gen-pass-decls -name Hir)
add_public_tablegen_target(MLIRHirPassIncGen)

add_subdirectory(Transforms)
