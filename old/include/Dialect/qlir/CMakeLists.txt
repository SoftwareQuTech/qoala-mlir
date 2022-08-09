# add_mlir_dialect(QLirOps qlir)


# function(add_mlir_dialect dialect dialect_namespace)
set(LLVM_TARGET_DEFINITIONS QLirOps.td)
mlir_tablegen(QLirOps.h.inc -gen-op-decls --typedefs-dialect=qlir)
mlir_tablegen(QLirOps.cpp.inc -gen-op-defs --typedefs-dialect=qlir)
mlir_tablegen(QLirOpsTypes.h.inc -gen-typedef-decls --typedefs-dialect=qlir)
mlir_tablegen(QLirOpsTypes.cpp.inc -gen-typedef-defs --typedefs-dialect=qlir)
mlir_tablegen(QLirOpsDialect.h.inc -gen-dialect-decls -dialect=qlir)
mlir_tablegen(QLirOpsDialect.cpp.inc -gen-dialect-defs -dialect=qlir)
add_public_tablegen_target(MLIRQLirOpsIncGen)
add_dependencies(mlir-headers MLIRQLirOpsIncGen)



add_mlir_doc(QLirDialect QLirDialect qlir/ -gen-dialect-doc)
add_mlir_doc(QLirTypes QLirTypes qlir/ -gen-op-doc)
add_mlir_doc(QLirOps QLirOps qlir/ -gen-op-doc)


