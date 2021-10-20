#ifndef HIR_OPS
#define HIR_OPS

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/hir/HirDialect.td"
include "Dialect/hir/HirTypes.td"


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


def AllocQubitOp : Hir_Op<"qalloc"> {
  let results = (outs Hir_QubitType:$qout);
  
  let assemblyFormat = [{ attr-dict `:` type($qout) }];
  let verifier = ?;
}

def GateXOp : Hir_Op<"gate_x"> {
  let arguments = (ins Hir_QubitType:$qin);
  let results = (outs Hir_QubitType:$qout);

  let assemblyFormat = [{ attr-dict $qin `=` }];
  let verifier = ?;
}

def GateYOp : Hir_Op<"gate_y"> {
  let arguments = (ins Hir_QubitType:$qin);
  let results = (outs Hir_QubitType:$qout);

  let assemblyFormat = [{ attr-dict $qin `=` }];
  let verifier = ?;
}



#endif // HIR_OPS
