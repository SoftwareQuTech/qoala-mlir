#ifndef LIR_DIALECT
#define LIR_DIALECT

include "mlir/IR/OpBase.td"

def Lir_Dialect : Dialect {
  let name = "lir";
  let cppNamespace = "::mlir::lir";
}

class Lir_Op<string mnemonic, list<OpTrait> traits = []> :
        Op<Lir_Dialect, mnemonic, traits> {

    let verifier = [{ return ::verify(*this); }];
}

class Lir_Type<string name> : TypeDef<Lir_Dialect, name> {}

#endif // LIR_DIALECT
