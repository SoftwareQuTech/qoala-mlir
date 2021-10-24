#ifndef HIR_TYPES
#define HIR_TYPES

include "Dialect/hir/HirDialect.td"

def Hir_QubitType : Hir_Type<"Qubit"> {
  let mnemonic = "qubit";
  let summary = "Qubit type";
}

def Hir_CValueType : Hir_Type<"CValue"> {
  let mnemonic = "cvalue";
  let summary = "Classical value type";
}



#endif // HIR_TYPES