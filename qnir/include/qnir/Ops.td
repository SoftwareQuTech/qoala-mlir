#ifndef QNIR_OPS
#define QNIR_OPS

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

def Qnir_Dialect : Dialect {
  let name = "qnir";
  let cppNamespace = "::mlir::qnir";
}

class Qnir_Op<string mnemonic, list<OpTrait> traits = []> :
    Op<Qnir_Dialect, mnemonic, traits>;

def ConstantOp : Qnir_Op<"constant", [NoSideEffect]> {
  let summary = "constant";

  let arguments = (ins F64ElementsAttr:$value);

  let results = (outs F64Tensor);

  let parser = [{ return ::parseConstantOp(parser, result); }];
  let printer = [{ return ::print(p, *this); }];

  let builders = [
    OpBuilder<(ins "DenseElementsAttr":$value), [{
      build($_builder, $_state, value.getType(), value);
    }]>,

    OpBuilder<(ins "double":$value)>
  ];

  let verifier = [{ return ::verify(*this); }];
}

def AddOp : Qnir_Op<"add"> {
  let summary = "element-wise addition operation";

  let arguments = (ins F64Tensor:$lhs, F64Tensor:$rhs);
  let results = (outs F64Tensor);

  let parser = [{ return ::parseBinaryOp(parser, result); }];
  let printer = [{ return ::printBinaryOp(p, *this); }];

  let builders = [
    OpBuilder<(ins "Value":$lhs, "Value":$rhs)>
  ];
}


def PrintOp : Qnir_Op<"print"> {
  let summary = "print operation";
  let arguments = (ins F64Tensor:$input);
  let assemblyFormat = "$input attr-dict `:` type($input)";
}

def Print2Op : Qnir_Op<"print2"> {
  let summary = "print2 operation";
  let arguments = (ins F64Tensor:$input);
  let assemblyFormat = "$input attr-dict `:` type($input)";
}


def ReturnOp : Qnir_Op<"return", [NoSideEffect, HasParent<"FuncOp">,
                                 Terminator]> {
  let summary = "return operation";
  let arguments = (ins Variadic<F64Tensor>:$input);

  let assemblyFormat = "($input^ `:` type($input))? attr-dict ";

  let builders = [
    OpBuilder<(ins), [{ build($_builder, $_state, llvm::None); }]>
  ];

  let extraClassDeclaration = [{
    bool hasOperand() { return getNumOperands() != 0; }
  }];

  let verifier = [{ return ::verify(*this); }];
}

#endif // QNIR_OPS
