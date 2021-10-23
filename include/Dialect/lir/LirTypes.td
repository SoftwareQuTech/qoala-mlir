#ifndef LIR_TYPES
#define LIR_TYPES

include "Dialect/lir/LirDialect.td"

def Lir_QubitType : Lir_Type<"Qubit"> {
  let mnemonic = "qubit";
  let summary = "Qubit type";
}

def Lir_CMsgType : Lir_Type<"CMsg"> {
  let mnemonic = "cmsg";
  let summary = "Classical message type";
}


#endif // LIR_TYPES