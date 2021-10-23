#ifndef HIR_OPS
#define HIR_OPS

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/hir/HirDialect.td"
include "Dialect/hir/HirTypes.td"

def Quantum_AllocateOp : Hir_Op<"alloc", [MemoryEffects<[MemRead]>]> {
    let arguments = (ins Variadic<Index>:$dynamicsize);
    let results = (outs Quantum_QubitType:$qout);

    let builders = [
      OpBuilder<(ins "int":$size), [{
        return build($_builder, $_state, ::mlir::hir::QubitType::get($_builder.getContext(), size), {});
      }]>
    ];

    let verifier = ?;
    let assemblyFormat = [{ ( `(` $dynamicsize^ `)` )? attr-dict `:` type($qout) }];
}




#endif // HIR_OPS
