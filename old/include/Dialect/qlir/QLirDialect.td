#ifndef QLIR_DIALECT
#define QLIR_DIALECT

include "mlir/IR/OpBase.td"

def QLir_Dialect : Dialect {
  let name = "qlir";
  let summary = "A standalone out-of-tree MQLIR dialect.";
  let description = [{
      This dialect is an example of an out-of-tree MQLIR dialect designed to
      illustrate the basic setup required to develop MQLIR-based tools without
      working inside of the LLVM source tree.
  }];
  let cppNamespace = "::mlir::qlir";
}

class QLir_Op<string mnemonic, list<OpTrait> traits = []> :
        Op<QLir_Dialect, mnemonic, traits> {

    // For each Quantum op, the following static functions need to be defined in
    // QLirOps.cpp:
    //
    // * static ParseResult parse<op-c++-class-name>(OpAsmParser &parser,
    //                                               OperationState &state);
    // * static void print(OpAsmPrinter &p, <op-c++-class-name> op)
    // * static LogicalResult verify(<op-c++-class-name> op)
    // let parser = [{ return ::parse$cppClass(parser, result); }];
    // let printer = [{ ::print(*this, p); }];
    let verifier = [{ return ::verify(*this); }];
}

class QLir_Type<string name> : TypeDef<QLir_Dialect, name> {}

#endif // QLIR_DIALECT
