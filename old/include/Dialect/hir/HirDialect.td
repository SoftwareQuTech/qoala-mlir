#ifndef HIR_DIALECT
#define HIR_DIALECT

include "mlir/IR/OpBase.td"

def Hir_Dialect : Dialect {
  let name = "hir";
  let cppNamespace = "::mlir::hir";

  // Used  so that "parseType" and "printType" declarations are generated!
  let useDefaultTypePrinterParser = 1;
}

class Hir_Op<string mnemonic, list<OpTrait> traits = []> :
        Op<Hir_Dialect, mnemonic, traits> {

    let verifier = [{ return ::verify(*this); }];
}

class Hir_Type<string name> : TypeDef<Hir_Dialect, name> {}

#endif // HIR_DIALECT
