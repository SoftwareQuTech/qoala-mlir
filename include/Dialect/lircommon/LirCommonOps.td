#ifndef LIRCOMMON_OPS
#define LIRCOMMON_OPS

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/lircommon/LirCommonDialect.td"
include "Dialect/lircommon/LirCommonTypes.td"

def LirCommon_AllocateOp : LirCommon_Op<"alloc"> {
    let results = (outs LirCommon_QubitType:$qout);
    let verifier = ?;
}

def LirCommon_EntangleOp : LirCommon_Op<"entangle"> {
    let results = (outs LirCommon_QubitType:$qout);
    let verifier = ?;
}

def LirCommon_NewCValueOnClasOp : LirCommon_Op<"new_cval_c"> {
    let results = (outs LirCommon_CValueOnClasType:$cout);
    let verifier = ?;
}

def LirCommon_NewCValueOnQuanOp : LirCommon_Op<"new_cval_q"> {
    let results = (outs LirCommon_CValueOnQuanType:$cout);
    let verifier = ?;
}

def LirCommon_SendCMsgOp : LirCommon_Op<"send_cmsg"> {
    let arguments = (ins LirCommon_CValueOnClasType:$cin);
    let verifier = ?;
}

def LirCommon_RecvCMsgOp : LirCommon_Op<"recv_cmsg"> {
    let results = (outs LirCommon_CValueOnClasType:$cout);
    let verifier = ?;
}

def GateXOp : LirCommon_Op<"gate_x"> {
  let arguments = (ins LirCommon_QubitType:$qin, LirCommon_CValueOnQuanType:$cin);
  let results = (outs LirCommon_QubitType:$qout);
  let verifier = ?;
}

def GateYOp : LirCommon_Op<"gate_y"> {
  let arguments = (ins LirCommon_QubitType:$qin, LirCommon_CValueOnQuanType:$cin);
  let results = (outs LirCommon_QubitType:$qout);
  let verifier = ?;
}

def GateZOp : LirCommon_Op<"gate_Z"> {
  let arguments = (ins LirCommon_QubitType:$qin, LirCommon_CValueOnQuanType:$cin);
  let results = (outs LirCommon_QubitType:$qout);
  let verifier = ?;
}

def CValueClassToQuanOp : LirCommon_Op<"cval_c_to_q"> {
  let arguments = (ins LirCommon_CValueOnClasType:$cin);
  let results = (outs LirCommon_CValueOnQuanType:$cout);
  let verifier = ?;
}

def CValueQuanToClasOp : LirCommon_Op<"cval_q_to_c"> {
  let arguments = (ins LirCommon_CValueOnQuanType:$cin);
  let results = (outs LirCommon_CValueOnClasType:$cout);
  let verifier = ?;
}

def MeasOp : LirCommon_Op<"meas"> {
  let arguments = (ins LirCommon_QubitType:$qin);
  let results = (outs LirCommon_CValueOnQuanType:$cout);
  let verifier = ?;
}

#endif // LIRCOMMON_OPS
