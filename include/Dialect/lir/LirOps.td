#ifndef LIR_OPS
#define LIR_OPS

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/lir/LirDialect.td"
include "Dialect/lir/LirTypes.td"

def AllocateOp : Lir_Op<"alloc"> {
    let results = (outs QubitType:$qout);
    let verifier = ?;
}

def EntangleOp : Lir_Op<"entangle"> {
    let results = (outs QubitType:$qout);
    let verifier = ?;
}

def SendCMsgOp : Lir_Op<"send_cmsg"> {
    let arguments = (ins CValueCType:$cin);
    let verifier = ?;
}

def NoResultOp : Lir_Op<"no_result"> {
    let verifier = ?;
}

def RecvCMsgOp : Lir_Op<"recv_cmsg"> {
    let results = (outs CValueCType:$cout);
    let verifier = ?;
}

def RotXOp : Lir_Op<"rot_x"> {
  let arguments = (ins QubitType:$qin, CValueQType:$cin);
  let results = (outs QubitType:$qout);
  let verifier = ?;
}

def RotYOp : Lir_Op<"rot_y"> {
  let arguments = (ins QubitType:$qin, CValueQType:$cin);
  let results = (outs QubitType:$qout);
  let verifier = ?;
}

def RotZOp : Lir_Op<"rot_z"> {
  let arguments = (ins QubitType:$qin, CValueQType:$cin);
  let results = (outs QubitType:$qout);
  let verifier = ?;
}

def GateXOp : Lir_Op<"gate_x"> {
  let arguments = (ins QubitType:$qin);
  let results = (outs QubitType:$qout);
  let verifier = ?;
}

def GateYOp : Lir_Op<"gate_y"> {
  let arguments = (ins QubitType:$qin);
  let results = (outs QubitType:$qout);
  let verifier = ?;
}

def GateZOp : Lir_Op<"gate_Z"> {
  let arguments = (ins QubitType:$qin);
  let results = (outs QubitType:$qout);
  let verifier = ?;
}

def GateHOp : Lir_Op<"gate_h"> {
  let arguments = (ins QubitType:$qin);
  let results = (outs QubitType:$qout);
  let verifier = ?;
}

def GateCphaseOp : Lir_Op<"cphase"> {
  let arguments = (ins QubitType:$qin0, QubitType:$qin1);
  let results = (outs QubitType:$qout0, QubitType:$qout1);
  let verifier = ?;
}

def GateCnotOp : Lir_Op<"cnot"> {
  let arguments = (ins QubitType:$qin0, QubitType:$qin1);
  let results = (outs QubitType:$qout0, QubitType:$qout1);
  let verifier = ?;
}

def MeasOp : Lir_Op<"meas"> {
  let arguments = (ins QubitType:$qin);
  let results = (outs CValueQType:$cout);
  let verifier = ?;
}

def NewCValueCOp : Lir_Op<"new_cval_c"> {
    let results = (outs CValueCType:$cout);
    let verifier = ?;
}

def NewCValueQOp : Lir_Op<"new_cval_q"> {
    let results = (outs CValueQType:$cout);
    let verifier = ?;
}

def CValueCtoQOp : Lir_Op<"cval_c_to_q"> {
  let arguments = (ins CValueCType:$cin);
  let results = (outs CValueQType:$cout);
  let verifier = ?;
}

def CValueQtoCOp : Lir_Op<"cval_q_to_c"> {
  let arguments = (ins CValueQType:$cin);
  let results = (outs CValueCType:$cout);
  let verifier = ?;
}

def AddCValueCOp : Lir_Op<"add_cvalue_c"> {
  let arguments = (ins CValueCType:$cin0, CValueCType:$cin1);
  let results = (outs CValueCType:$cout);
  let verifier = ?;

  let builders = [
    OpBuilder<(ins "Value":$cin0, "Value":$cin1), [{
      $_state.addOperands(cin0);
      $_state.addOperands(cin1);
      $_state.addTypes(CValueCType::get($_builder.getContext()));
    }]>
  ];
}

def AddCValueQOp : Lir_Op<"add_cvalue_q"> {
  let arguments = (ins CValueQType:$cin0, CValueQType:$cin1);
  let results = (outs CValueQType:$cout);
  let verifier = ?;

  let builders = [
    OpBuilder<(ins "Value":$cin0, "Value":$cin1), [{
      $_state.addOperands(cin0);
      $_state.addOperands(cin1);
      $_state.addTypes(CValueQType::get($_builder.getContext()));
    }]>
  ];
}

#endif // LIR_OPS
