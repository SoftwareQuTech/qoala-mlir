#ifndef HIR_TYPES
#define HIR_TYPES

include "Dialect/hir/HirDialect.td"

def Hir_QubitType : Hir_Type<"Qubit"> {
  let mnemonic = "qubit";
  let summary = "Qubit type";
}

def Hir_CMsgType : Hir_Type<"CMsg"> {
  let mnemonic = "cmsg";
  let summary = "Classical message type";
}


#endif // HIR_TYPES