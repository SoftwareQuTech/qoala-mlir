#ifndef LIR_TYPES_TD
#define LIR_TYPES_TD

include "mlir/IR/AttrTypeBase.td"

include "Dialect/lir/LirDialect.td"

class Lir_Type<string name, string typeMnemonic, list<Trait> traits = []> 
    : TypeDef<Lir_Dialect, name, traits> {
  let mnemonic = typeMnemonic;
}

def CommQubitType: Lir_Type<"CommQubit", "cqubit">;
def QubitType: Lir_Type<"Qubit", "qubit">;
def EntQubitType: Lir_Type<"EntQubit", "equbit">;

def NoisyQubitType: Lir_Type<"NoisyQubit", "nqubit"> {
  let parameters = (ins "unsigned":$fidelity);

  let assemblyFormat = "`<` `fid` `=` $fidelity `>`";
}

#endif // LIR_TYPES_TD