#ifndef LIRCOMMON_DIALECT
#define LIRCOMMON_DIALECT

include "mlir/IR/OpBase.td"

def LirCommon_Dialect : Dialect {
  let name = "lircommon";
  let summary = "A standalone out-of-tree MLIRCOMMON dialect.";
  let description = [{
      This dialect is an example of an out-of-tree MLIRCOMMON dialect designed to
      illustrate the basic setup required to develop MLIRCOMMON-based tools without
      working inside of the LLVM source tree.
  }];
  let cppNamespace = "::mlir::lircommon";
}

class LirCommon_Op<string mnemonic, list<OpTrait> traits = []> :
        Op<LirCommon_Dialect, mnemonic, traits> {

    // For each Quantum op, the following static functions need to be defined in
    // LirCommonOps.cpp:
    //
    // * static ParseResult parse<op-c++-class-name>(OpAsmParser &parser,
    //                                               OperationState &state);
    // * static void print(OpAsmPrinter &p, <op-c++-class-name> op)
    // * static LogicalResult verify(<op-c++-class-name> op)
    // let parser = [{ return ::parse$cppClass(parser, result); }];
    // let printer = [{ ::print(*this, p); }];
    let verifier = [{ return ::verify(*this); }];
}

class LirCommon_Type<string name> : TypeDef<LirCommon_Dialect, name> {}

#endif // LIRCOMMON_DIALECT
