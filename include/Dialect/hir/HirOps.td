#ifndef HIR_OPS
#define HIR_OPS

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/hir/HirDialect.td"
include "Dialect/hir/HirTypes.td"

def Hir_AllocateOp : Hir_Op<"alloc"> {
    let results = (outs Hir_QubitType:$qout);

    let verifier = ?;
}

def Hir_EntangleOp : Hir_Op<"entangle"> {
    let results = (outs Hir_QubitType:$qout);

    let verifier = ?;
}

def Hir_NewCValueOp : Hir_Op<"new_cval"> {
    let results = (outs Hir_CValueType:$cout);

    let verifier = ?;
}

def Hir_SendCMsgOp : Hir_Op<"send_cmsg"> {
    let arguments = (ins Hir_CValueType:$cin);

    let verifier = ?;
}

def Hir_RecvCMsgOp : Hir_Op<"recv_cmsg"> {
    let results = (outs Hir_CValueType:$cout);

    let verifier = ?;
}


def GateXOp : Hir_Op<"gate_x"> {
  let arguments = (ins Hir_QubitType:$qin, Hir_CValueType:$cin);
  let results = (outs Hir_QubitType:$qout);

  let verifier = ?;
}

def GateYOp : Hir_Op<"gate_y"> {
  let arguments = (ins Hir_QubitType:$qin, Hir_CValueType:$cin);
  let results = (outs Hir_QubitType:$qout);

  let verifier = ?;
}

def GateZOp : Hir_Op<"gate_z"> {
  let arguments = (ins Hir_QubitType:$qin, Hir_CValueType:$cin);
  let results = (outs Hir_QubitType:$qout);

  let verifier = ?;
}

def MeasOp : Hir_Op<"meas"> {
  let arguments = (ins Hir_QubitType:$qin);
  let results = (outs Hir_CValueType:$cout);

  let verifier = ?;
}



#endif // HIR_OPS
