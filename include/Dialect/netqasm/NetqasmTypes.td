#ifndef NETQASM_TYPES_TD
#define NETQASM_TYPES_TD

include "mlir/IR/AttrTypeBase.td"

include "Dialect/netqasm/NetqasmDialect.td"

class Netqasm_Type<string name, string typeMnemonic, list<Trait> traits = []> 
    : TypeDef<Netqasm_Dialect, name, traits> {
  let mnemonic = typeMnemonic;
}

def VirtualQubitType: Netqasm_Type<"VirtualQubit", "vqubit">;

#endif // NETQASM_TYPES_TD