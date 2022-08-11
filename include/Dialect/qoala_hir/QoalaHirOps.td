#ifndef QOALA_HIR_OPS_TD
#define QOALA_HIR_OPS_TD

include "mlir/IR/OpBase.td"
include "Dialect/qoala_hir/QoalaHirDialect.td"

// Needed for `[NoSideEffect]`
// Note that QoalaHir.h also needs to include "mlir/Interfaces/SideEffectInterfaces.h" !
include "mlir/Interfaces/SideEffectInterfaces.td"

class QoalaHir_Op<string mnemonic, list<Trait> traits = []> :
  Op<QoalaHir_Dialect, mnemonic, traits>;

class QoalaHir_QuantumOp<string mnemonic, list<Trait> traits = []> :
  QoalaHir_Op<mnemonic, traits>;

class QoalaHir_ClassicalOp<string mnemonic, list<Trait> traits = []> :
  QoalaHir_Op<mnemonic, traits>;


def ConstantOp : QoalaHir_Op<"constant"> {
    let arguments = (ins I32Attr:$value);

    let results = (outs I32);
}


def NewQubitOp : QoalaHir_QuantumOp<"new_qubit"> {
    let results = (outs F64:$qout);
}

def EntangleKeepOp : QoalaHir_QuantumOp<"entangle_keep"> {
    let results = (outs F64:$qout, I1:$success);
}

def EntangleMeasureOp : QoalaHir_QuantumOp<"entangle_measure"> {
    let results = (outs F64:$qout);
}

def SendMsgOp : QoalaHir_ClassicalOp<"send_msg"> {
    let arguments = (ins I32:$cin);
}

def ReceiveMsgOp : QoalaHir_ClassicalOp<"receive_msg"> {
    let results = (outs I32:$cout);
}

def AngleGateOp : QoalaHir_QuantumOp<"angle_gate"> {
  let arguments = (ins F64:$qin, F64:$cin);
  let results = (outs F64:$qout);
}

def GateOp : QoalaHir_QuantumOp<"gate"> {
  let arguments = (ins F64:$qin);
  let results = (outs F64:$qout);
}

def TwoQubitGateOp : QoalaHir_QuantumOp<"two_qubit_gate"> {
  let arguments = (ins F64:$qin0, F64:$qin1);
  let results = (outs F64:$qout0, F64:$qout1);
}

def MeasureOp : QoalaHir_QuantumOp<"measure"> {
  let arguments = (ins F64:$qin);
  let results = (outs F64:$cout);
}

def AddOp : QoalaHir_ClassicalOp<"add"> {
  let arguments = (ins I32:$cin0, I32:$cin1);
  let results = (outs I32:$cout);
}

def MultiplyConstOp : QoalaHir_ClassicalOp<"multiply_const"> {
  let arguments = (ins I32:$cin0, I32Attr:$constant);
  let results = (outs I32:$cout);
}

def ConditionalMultiplyConstOp : QoalaHir_ClassicalOp<"conditional_multiply_const"> {
  let arguments = (ins I32:$cin0, I32:$cin1, I32Attr:$constant);
  let results = (outs I32:$cout);
}

#endif // QOALA_HIR_OPS_TD