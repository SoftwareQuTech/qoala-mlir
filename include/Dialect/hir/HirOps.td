#ifndef HIR_OPS
#define HIR_OPS

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/hir/HirDialect.td"
include "Dialect/hir/HirTypes.td"


def GateOp : Hir_Op<"gate"> {
  let arguments = (ins F32:$in_qubit);

  let results = (outs F32:$out_qubit);
}

// def NewValueOp : Hir_Op<"new_value"> {
//   let results = (outs F32:$value);
// }

// def OneOperandOp : Hir_Op<"one_operand"> {
//   let arguments = (ins F32:$value);
// }

// def ConstantOp : Hir_Op<"constant", [NoSideEffect]> {
//   let arguments = (ins F64ElementsAttr:$value);

//   let results = (outs F64Tensor);

//   let parser = [{ return ::parseConstantOp(parser, result); }];
//   let printer = [{ return ::print(p, *this); }];

//   let builders = [
//     OpBuilder<(ins "DenseElementsAttr":$value), [{
//       build($_builder, $_state, value.getType(), value);
//     }]>,

//     OpBuilder<(ins "double":$value)>
//   ];

//   let verifier = [{ return ::verify(*this); }];
// }

def UseQubitOp : Hir_Op<"use_qubit"> {
  let arguments = (ins Hir_QubitType);
}


#endif // HIR_OPS
