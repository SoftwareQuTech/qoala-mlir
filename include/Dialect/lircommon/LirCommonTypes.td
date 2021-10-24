#ifndef LIRCOMMON_TYPES
#define LIRCOMMON_TYPES

include "Dialect/lircommon/LirCommonDialect.td"

def LirCommon_CValueOnClasType : LirCommon_Type<"CValueOnClas"> {
  let mnemonic = "cvalue_on_clas";
  let summary = "Classical value on CPU";
}

def LirCommon_CValueOnQuanType : LirCommon_Type<"CValueOnQuan"> {
  let mnemonic = "cvalue_on_quan";
  let summary = "Classical value on QPU";
}

def LirCommon_QubitType : LirCommon_Type<"Qubit"> {
  let mnemonic = "qubit";
  let summary = "Qubit type";
}


#endif // LIRCOMMON_TYPES