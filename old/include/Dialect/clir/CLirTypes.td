#ifndef CLIR_TYPES
#define CLIR_TYPES

include "Dialect/clir/CLirDialect.td"

def CLir_CMsgType : CLir_Type<"CMsg"> {
  let mnemonic = "cmsg";
  let summary = "Classical message type";
}

def CLir_ValueType : CLir_Type<"CLirValue"> {
  let mnemonic = "value";
  let summary = "Classical value type";
}


#endif // CLIR_TYPES