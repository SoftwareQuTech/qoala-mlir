#ifndef HIR_OPS
#define HIR_OPS

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

def Hir_Dialect : Dialect {
  let name = "hir";
  let cppNamespace = "::mlir::hir";
}

class Hir_Op<string mnemonic, list<OpTrait> traits = []> :
    Op<Hir_Dialect, mnemonic, traits>;

class Hir_Type<string name> : TypeDef<Hir_Dialect, name> { }

def QubitType : Hir_Type<"Qubit"> {
  let parameters = (ins "int":$intParam);
  let printer = [{ ::print(*this, $_printer); }];
  let parser = [{ return ::parseQubit($_printer, $_ctxt); }];
}
  
def GateOp : Hir_Op<"gate"> {
  let arguments = (ins F32:$in_qubit);

  let results = (outs F32:$out_qubit);
}

def NewValueOp : Hir_Op<"new_value"> {
  let results = (outs F32:$value);
}

def OneOperandOp : Hir_Op<"one_operand"> {
  let arguments = (ins F32:$value);
}

def ConstantOp : Hir_Op<"constant", [NoSideEffect]> {
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

def AllocQubitOp : Hir_Op<"qalloc"> {
  let results = (outs QubitType);
}


#endif // HIR_OPS
