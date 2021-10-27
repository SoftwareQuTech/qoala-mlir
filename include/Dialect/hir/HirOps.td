#ifndef HIR_OPS
#define HIR_OPS

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/hir/HirDialect.td"
include "Dialect/hir/HirTypes.td"

def AllocateOp : Hir_Op<"alloc"> {
    let results = (outs QubitType:$qout);
    let verifier = ?;
}

def EntangleOp : Hir_Op<"entangle"> {
    let results = (outs QubitType:$qout);
    let verifier = ?;
}

def NewCValueOp : Hir_Op<"new_cval"> {
    let results = (outs CValueType:$cout);
    let verifier = ?;
}

def SendCMsgOp : Hir_Op<"send_cmsg"> {
    let arguments = (ins CValueType:$cin);
    let verifier = ?;
}

def RecvCMsgOp : Hir_Op<"recv_cmsg"> {
    let results = (outs CValueType:$cout);
    let verifier = ?;
}

def RotXOp : Hir_Op<"rot_x"> {
  let arguments = (ins QubitType:$qin, CValueType:$cin);
  let results = (outs QubitType:$qout);
  let verifier = ?;
}

def RotYOp : Hir_Op<"rot_y"> {
  let arguments = (ins QubitType:$qin, CValueType:$cin);
  let results = (outs QubitType:$qout);
  let verifier = ?;
}

def RotZOp : Hir_Op<"rot_z"> {
  let arguments = (ins QubitType:$qin, CValueType:$cin);
  let results = (outs QubitType:$qout);
  let verifier = ?;
}

def GateXOp : Hir_Op<"gate_x"> {
  let arguments = (ins QubitType:$qin);
  let results = (outs QubitType:$qout);
  let verifier = ?;
}

def GateYOp : Hir_Op<"gate_y"> {
  let arguments = (ins QubitType:$qin);
  let results = (outs QubitType:$qout);
  let verifier = ?;
}

def GateZOp : Hir_Op<"gate_z"> {
  let arguments = (ins QubitType:$qin);
  let results = (outs QubitType:$qout);
  let verifier = ?;
}

def GateHOp : Hir_Op<"gate_h"> {
  let arguments = (ins QubitType:$qin);
  let results = (outs QubitType:$qout);
  let verifier = ?;
}

def GateCphaseOp : Hir_Op<"cphase"> {
  let arguments = (ins QubitType:$qin0, QubitType:$qin1);
  let results = (outs QubitType:$qout0, QubitType:$qout1);
  let verifier = ?;
}

def GateCnotOp : Hir_Op<"cnot"> {
  let arguments = (ins QubitType:$qin0, QubitType:$qin1);
  let results = (outs QubitType:$qout0, QubitType:$qout1);
  let verifier = ?;
}

def MeasOp : Hir_Op<"meas"> {
  let arguments = (ins QubitType:$qin);
  let results = (outs CValueType:$cout);
  let verifier = ?;
}



#endif // HIR_OPS
