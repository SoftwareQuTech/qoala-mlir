#ifndef QNET_TYPES_TD
#define QNET_TYPES_TD

include "mlir/IR/AttrTypeBase.td"

include "Dialect/Qnet/QnetDialect.td"

class Qnet_Type<string name, string typeMnemonic, list<Trait> traits = []>
    : TypeDef<Qnet_Dialect, name, traits> {
  let mnemonic = typeMnemonic;
}

def QubitType: Qnet_Type<"Qubit", "qubit">;


#endif // QNET_TYPES_TD