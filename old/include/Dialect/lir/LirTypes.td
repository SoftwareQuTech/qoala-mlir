#ifndef LIR_TYPES
#define LIR_TYPES

include "Dialect/lir/LirDialect.td"

def CValueCType : Lir_Type<"CValueC"> {
  let mnemonic = "cvalue_c";
  let summary = "Classical value on CPU";
}

def CValueQType : Lir_Type<"CValueQ"> {
  let mnemonic = "cvalue_q";
  let summary = "Classical value on QPU";
}

def QubitType : Lir_Type<"Qubit"> {
  let mnemonic = "qubit";
  let summary = "Qubit type";
}


#endif // LIR_TYPES