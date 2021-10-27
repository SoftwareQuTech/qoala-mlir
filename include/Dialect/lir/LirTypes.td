#ifndef LIR_TYPES
#define LIR_TYPES

include "Dialect/lir/LirDialect.td"

def Lir_CValueOnClasType : Lir_Type<"CValueOnClas"> {
  let mnemonic = "cvalue_on_clas";
  let summary = "Classical value on CPU";
}

def Lir_CValueOnQuanType : Lir_Type<"CValueOnQuan"> {
  let mnemonic = "cvalue_on_quan";
  let summary = "Classical value on QPU";
}

def Lir_QubitType : Lir_Type<"Qubit"> {
  let mnemonic = "qubit";
  let summary = "Qubit type";
}


#endif // LIR_TYPES