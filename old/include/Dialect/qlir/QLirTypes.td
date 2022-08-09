#ifndef QLIR_TYPES
#define QLIR_TYPES

include "Dialect/qlir/QLirDialect.td"

def QLir_QubitType : QLir_Type<"Qubit"> {
  let mnemonic = "qubit";
  let summary = "Qubit type";
}

def QLir_CValueType : QLir_Type<"CValue"> {
  let mnemonic = "cvalue";
  let summary = "Classical value type";
}



#endif // QLIR_TYPES