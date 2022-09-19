#ifndef QOALA_LIR_M_TYPES_TD
#define QOALA_LIR_M_TYPES_TD

include "mlir/IR/AttrTypeBase.td"

include "Dialect/qoala_lir_m/QoalaLirMDialect.td"

class QoalaLirM_Type<string name, string typeMnemonic, list<Trait> traits = []> 
    : TypeDef<QoalaLirM_Dialect, name, traits> {
  let mnemonic = typeMnemonic;
}

def CommQubitType: QoalaLirM_Type<"CommQubit", "comm_qubit">;

#endif // QOALA_LIR_M_TYPES_TD