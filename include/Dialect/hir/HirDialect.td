#ifndef HIR_DIALECT
#define HIR_DIALECT

include "mlir/IR/OpBase.td"

def Hir_Dialect : Dialect {
  let name = "hir";
  let cppNamespace = "::mlir::hir";
}

class Hir_Op<string mnemonic, list<OpTrait> traits = []> :
        Op<Hir_Dialect, mnemonic, traits> {

    // For each Quantum op, the following static functions need to be defined in
    // HirOps.cpp:
    //
    // * static ParseResult parse<op-c++-class-name>(OpAsmParser &parser,
    //                                               OperationState &state);
    // * static void print(OpAsmPrinter &p, <op-c++-class-name> op)
    // * static LogicalResult verify(<op-c++-class-name> op)
    // let parser = [{ return ::parse$cppClass(parser, result); }];
    // let printer = [{ ::print(*this, p); }];
    // let verifier = [{ return ::verify(*this); }];
}

class Hir_Type<string name> : TypeDef<Hir_Dialect, name> {}

#endif // HIR_DIALECT
