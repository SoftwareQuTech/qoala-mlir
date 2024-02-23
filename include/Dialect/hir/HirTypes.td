#ifndef HIR_TYPES_TD
#define HIR_TYPES_TD

include "mlir/IR/AttrTypeBase.td"

include "Dialect/hir/HirDialect.td"

class Hir_Type<string name, string typeMnemonic, list<Trait> traits = []> 
    : TypeDef<Hir_Dialect, name, traits> {
  let mnemonic = typeMnemonic;
}

def QubitType: Hir_Type<"Qubit", "qubit">;


#endif // HIR_TYPES_TD