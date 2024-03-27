#ifndef QNET_TYPES_TD
#define QNET_TYPES_TD

include "mlir/IR/AttrTypeBase.td"

include "Dialect/QNet/QNetDialect.td"

class QNet_Type<string name, string typeMnemonic, list<Trait> traits = []>
    : TypeDef<QNet_Dialect, name, traits> {
  let mnemonic = typeMnemonic;
}

def QubitType: QNet_Type<"Qubit", "qubit">;


#endif // QNET_TYPES_TD