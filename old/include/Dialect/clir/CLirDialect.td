#ifndef CLIR_DIALECT
#define CLIR_DIALECT

include "mlir/IR/OpBase.td"

def CLir_Dialect : Dialect {
  let name = "clir";
  let summary = "A standalone out-of-tree MCLIR dialect.";
  let description = [{
      This dialect is an example of an out-of-tree MCLIR dialect designed to
      illustrate the basic setup required to develop MCLIR-based tools without
      working inside of the LLVM source tree.
  }];
  let cppNamespace = "::mlir::clir";
}

class CLir_Op<string mnemonic, list<OpTrait> traits = []> :
        Op<CLir_Dialect, mnemonic, traits> {

    // For each Quantum op, the following static functions need to be defined in
    // CLirOps.cpp:
    //
    // * static ParseResult parse<op-c++-class-name>(OpAsmParser &parser,
    //                                               OperationState &state);
    // * static void print(OpAsmPrinter &p, <op-c++-class-name> op)
    // * static LogicalResult verify(<op-c++-class-name> op)
    // let parser = [{ return ::parse$cppClass(parser, result); }];
    // let printer = [{ ::print(*this, p); }];
    let verifier = [{ return ::verify(*this); }];
}

class CLir_Type<string name> : TypeDef<CLir_Dialect, name> {}

#endif // CLIR_DIALECT
