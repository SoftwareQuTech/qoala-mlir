#ifndef HOST_TYPES_TD
#define HOST_TYPES_TD

include "mlir/IR/AttrTypeBase.td"

include "Dialect/Host/IR/HostDialect.td"

class Host_Type<string name, string typeMnemonic, list<Trait> traits = []> 
    : TypeDef<Host_Dialect, name, traits> {
  let mnemonic = typeMnemonic;
}

def CommQubitType: Host_Type<"CommQubit", "comm_qubit">;

#endif // HOST_TYPES_TD