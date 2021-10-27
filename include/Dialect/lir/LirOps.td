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

def Lir_NewCValueOnClasOp : Lir_Op<"new_cval_c"> {
    let results = (outs Lir_CValueOnClasType:$cout);
    let verifier = ?;
}

def Lir_NewCValueOnQuanOp : Lir_Op<"new_cval_q"> {
    let results = (outs Lir_CValueOnQuanType:$cout);
    let verifier = ?;
}

def Lir_SendCMsgOp : Lir_Op<"send_cmsg"> {
    let arguments = (ins Lir_CValueOnClasType:$cin);
    let verifier = ?;
}

def Lir_NoResultOp : Lir_Op<"no_result"> {
    let verifier = ?;
}

def Lir_RecvCMsgOp : Lir_Op<"recv_cmsg"> {
    let results = (outs Lir_CValueOnClasType:$cout);
    let verifier = ?;
}

def GateXOp : Lir_Op<"gate_x"> {
  let arguments = (ins Lir_QubitType:$qin, Lir_CValueOnQuanType:$cin);
  let results = (outs Lir_QubitType:$qout);
  let verifier = ?;
}

def GateYOp : Lir_Op<"gate_y"> {
  let arguments = (ins Lir_QubitType:$qin, Lir_CValueOnQuanType:$cin);
  let results = (outs Lir_QubitType:$qout);
  let verifier = ?;
}

def GateZOp : Lir_Op<"gate_z"> {
  let arguments = (ins Lir_QubitType:$qin, Lir_CValueOnQuanType:$cin);
  let results = (outs Lir_QubitType:$qout);
  let verifier = ?;
}

def GateHOp : Lir_Op<"gate_h"> {
  let arguments = (ins Lir_QubitType:$qin);
  let results = (outs Lir_QubitType:$qout);
  let verifier = ?;
}

def CPhaseOp : Lir_Op<"cphase"> {
  let arguments = (ins Lir_QubitType:$qin0, Lir_QubitType:$qin1);
  let results = (outs Lir_QubitType:$qout0, Lir_QubitType:$qout1);
  let verifier = ?;
}

def CNotOp : Lir_Op<"cnot"> {
  let arguments = (ins Lir_QubitType:$qin0, Lir_QubitType:$qin1);
  let results = (outs Lir_QubitType:$qout0, Lir_QubitType:$qout1);
  let verifier = ?;
}

def CValueClassToQuanOp : Lir_Op<"cval_c_to_q"> {
  let arguments = (ins Lir_CValueOnClasType:$cin);
  let results = (outs Lir_CValueOnQuanType:$cout);
  let verifier = ?;
}

def CValueQuanToClasOp : Lir_Op<"cval_q_to_c"> {
  let arguments = (ins Lir_CValueOnQuanType:$cin);
  let results = (outs Lir_CValueOnClasType:$cout);
  let verifier = ?;
}

def MeasOp : Lir_Op<"meas"> {
  let arguments = (ins Lir_QubitType:$qin);
  let results = (outs Lir_CValueOnQuanType:$cout);
  let verifier = ?;
}

def AddCValueOnQuanOp : Lir_Op<"add_cvalue_on_q"> {
  let arguments = (ins Lir_CValueOnQuanType:$cin0, Lir_CValueOnQuanType:$cin1);
  let results = (outs Lir_CValueOnQuanType:$cout);
  let verifier = ?;

  let builders = [
    OpBuilder<(ins "Value":$cin0, "Value":$cin1), [{
      $_state.addOperands(cin0);
      $_state.addOperands(cin1);
      $_state.addTypes(CValueOnQuanType::get(odsBuilder.getContext()));
    }]>
  ];
}

#endif // LIR_OPS
