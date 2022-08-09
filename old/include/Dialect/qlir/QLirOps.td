#ifndef QLIR_OPS
#define QLIR_OPS

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/qlir/QLirDialect.td"
include "Dialect/qlir/QLirTypes.td"

def QLir_AllocateOp : QLir_Op<"alloc"> {
    let results = (outs QLir_QubitType:$qout);

    let verifier = ?;
}

def QLir_EntangleOp : QLir_Op<"entangle"> {
    let results = (outs QLir_QubitType:$qout);

    let verifier = ?;
}

def QLir_RecvCLirValueOp : QLir_Op<"recv_clir"> {
    let arguments = (outs QLir_CValueType:$cval);
    let verifier = ?;
}

def QLir_SendCLirValueOp : QLir_Op<"send_clir"> {
    let arguments = (ins QLir_CValueType:$cval);
    let verifier = ?;
}



def RotXOp : QLir_Op<"rot_x"> {
  let arguments = (ins QLir_QubitType:$qin);
  let results = (outs QLir_QubitType:$qout);

  let verifier = ?;
}

def RotYOp : QLir_Op<"rot_y"> {
  let arguments = (ins QLir_QubitType:$qin);
  let results = (outs QLir_QubitType:$qout);

  let verifier = ?;
}



#endif // QLIR_OPS
