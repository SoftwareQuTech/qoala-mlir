#ifndef LIR_OPS
#define LIR_OPS

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/lir/LirDialect.td"
include "Dialect/lir/LirTypes.td"

def Lir_AllocateOp : Lir_Op<"alloc"> {
    let results = (outs Lir_QubitType:$qout);

    let verifier = ?;
}

def Lir_EntangleOp : Lir_Op<"entangle"> {
    let results = (outs Lir_QubitType:$qout);

    let verifier = ?;
}

def Lir_SendCMsgOp : Lir_Op<"send_cmsg"> {
    let arguments = (ins Lir_CMsgType:$cmout);

    let verifier = ?;
}

def Lir_RecvCMsgOp : Lir_Op<"recv_cmsg"> {
    let results = (outs Lir_CMsgType:$cmin);

    let verifier = ?;
}


def GateXOp : Lir_Op<"gate_x"> {
  let arguments = (ins Lir_QubitType:$qin);
  let results = (outs Lir_QubitType:$qout);

  let verifier = ?;
}

def GateYOp : Lir_Op<"gate_y"> {
  let arguments = (ins Lir_QubitType:$qin);
  let results = (outs Lir_QubitType:$qout);

  let verifier = ?;
}



#endif // LIR_OPS
