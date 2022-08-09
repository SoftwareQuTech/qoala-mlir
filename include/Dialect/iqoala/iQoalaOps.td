#ifndef IQOALA_OPS_TD
#define IQOALA_OPS_TD

include "mlir/IR/OpBase.td"
include "Dialect/iqoala/iQoalaDialect.td"

// Needed for `[NoSideEffect]`
// Note that iQoala.h also needs to include "mlir/Interfaces/SideEffectInterfaces.h" !
include "mlir/Interfaces/SideEffectInterfaces.td"

class iQoala_Op<string mnemonic, list<Trait> traits = []> :
  Op<iQoala_Dialect, mnemonic, traits>;

class iQoala_QuantumOp<string mnemonic, list<Trait> traits = []> :
  iQoala_Op<mnemonic, traits>;

class iQoala_ClassicalOp<string mnemonic, list<Trait> traits = []> :
  iQoala_Op<mnemonic, traits>;


def ConstantOp : iQoala_Op<"constant"> {
    let arguments = (ins F64ElementsAttr:$idk);

    let results = (outs F64Tensor);

    let hasVerifier = 1;

    let builders = [
        OpBuilder<(ins "DenseElementsAttr":$idk), [{
            build(odsBuilder, odsState, idk.getType(), idk);
        }]>,

        OpBuilder<(ins "double":$value)>
    ];
}

def TransposeOp : iQoala_Op<"transpose", [NoSideEffect]> {
  let summary = "transpose operation";

  let arguments = (ins F64Tensor:$input);
  let results = (outs F64Tensor);

  let assemblyFormat = [{
    `(` $input `:` type($input) `)` attr-dict `to` type(results)
  }];

  let builders = [
    OpBuilder<(ins "Value":$input)>
  ];

  let hasCanonicalizer = 1;
}

def ReshapeOp : iQoala_Op<"reshape", [NoSideEffect]> {
  let arguments = (ins F64Tensor:$input);

  let results = (outs StaticShapeTensorOf<[F64]>);

  let assemblyFormat = [{
    `(` $input `:` type($input) `)` attr-dict `to` type(results)
  }];

  let hasCanonicalizer = 1;
}


def QInitOp : iQoala_QuantumOp<"qinit"> {
    let results = (outs F64:$qout);
}

def EntangleKeepOp : iQoala_QuantumOp<"entangle_keep"> {
    let results = (outs F64:$qout);
}

def EntangleMeasureOp : iQoala_QuantumOp<"entangle_measure"> {
    let results = (outs F64:$qout);
}

def SendMsgOp : iQoala_ClassicalOp<"send_msg"> {
    let arguments = (ins I32:$cin);
}

def ReceiveMsgOp : iQoala_ClassicalOp<"receive_msg"> {
    let results = (outs I32:$cout);
}

def RotXOp : iQoala_QuantumOp<"rot_x"> {
  let arguments = (ins F64:$qin, F64:$cin);
  let results = (outs F64:$qout);
}

def RotYOp : iQoala_QuantumOp<"rot_y"> {
  let arguments = (ins F64:$qin, F64:$cin);
  let results = (outs F64:$qout);
}

def RotZOp : iQoala_QuantumOp<"rot_z"> {
  let arguments = (ins F64:$qin, F64:$cin);
  let results = (outs F64:$qout);
}

def GateXOp : iQoala_QuantumOp<"gate_x"> {
  let arguments = (ins F64:$qin);
  let results = (outs F64:$qout);
}

def GateYOp : iQoala_QuantumOp<"gate_y"> {
  let arguments = (ins F64:$qin);
  let results = (outs F64:$qout);
}

def GateZOp : iQoala_QuantumOp<"gate_Z"> {
  let arguments = (ins F64:$qin);
  let results = (outs F64:$qout);
}

def GateHOp : iQoala_QuantumOp<"gate_h"> {
  let arguments = (ins F64:$qin);
  let results = (outs F64:$qout);
}

def GateCphaseOp : iQoala_QuantumOp<"cphase"> {
  let arguments = (ins F64:$qin0, F64:$qin1);
  let results = (outs F64:$qout0, F64:$qout1);
}

def GateCnotOp : iQoala_QuantumOp<"cnot"> {
  let arguments = (ins F64:$qin0, F64:$qin1);
  let results = (outs F64:$qout0, F64:$qout1);
}

def MeasOp : iQoala_QuantumOp<"meas"> {
  let arguments = (ins F64:$qin);
  let results = (outs F64:$cout);
}

def HostConstantOp : iQoala_ClassicalOp<"host_const"> {
  let arguments = (ins I32Attr:$value);

  let results = (outs I32);
}

def QnosConstantOp : iQoala_ClassicalOp<"qnos_const"> {
  let results = (outs I32);
}

def HostAddOp : iQoala_ClassicalOp<"host_add"> {
  let arguments = (ins I32:$cin0, I32:$cin1);
  let results = (outs I32:$cout);
}

def QnosAdd : iQoala_QuantumOp<"qnos_add"> {
  let arguments = (ins F64:$cin0, F64:$cin1);
  let results = (outs F64:$cout);
}

#endif // IQOALA_OPS_TD