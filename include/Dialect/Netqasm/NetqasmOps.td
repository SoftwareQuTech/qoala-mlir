#ifndef NETQASM_OPS_TD
#define NETQASM_OPS_TD

include "mlir/IR/OpBase.td"

// Needed for FunctionOpInterface
// Note that Netqasm.h also needs to include "mlir/Interfaces/FunctionInterfaces.h" !
include "mlir/Interfaces/FunctionInterfaces.td"

include "mlir/IR/SymbolInterfaces.td"
include "mlir/Bytecode/BytecodeOpInterface.td"
include "mlir/Interfaces/CallInterfaces.td"
include "mlir/Interfaces/CastInterfaces.td"
include "mlir/Interfaces/ControlFlowInterfaces.td"
include "mlir/Interfaces/InferTypeOpInterface.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "mlir/IR/OpAsmInterface.td"

// Needed for `[NoMemoryEffect]`
// Note that Netqasm.h also needs to include "mlir/Interfaces/SideEffectInterfaces.h" !
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/Netqasm/NetqasmDialect.td"
include "Dialect/Netqasm/NetqasmTypes.td"


class Netqasm_Op<string mnemonic, list<Trait> traits = []> :
  Op<Netqasm_Dialect, mnemonic, traits>;

def DeclareVirtualQubitOp : Netqasm_Op<"decl_vqubit"> {
  let arguments = (ins I32Attr:$addr);
  let results = (outs VirtualQubitType:$q);
}

def InitOp : Netqasm_Op<"init"> {
  let arguments = (ins VirtualQubitType:$q);
}

def RotateXOp : Netqasm_Op<"rot_x"> {
  let arguments = (ins VirtualQubitType:$q, I32:$angle_num, I32:$angle_denom);
}

def RotateYOp : Netqasm_Op<"rot_y"> {
  let arguments = (ins VirtualQubitType:$q, I32:$angle_num, I32:$angle_denom);
}

def RotateZOp : Netqasm_Op<"rot_z"> {
  let arguments = (ins VirtualQubitType:$q, I32:$angle_num, I32:$angle_denom);
}

def HadamardOp : Netqasm_Op<"hadamard"> {
  let arguments = (ins VirtualQubitType:$q);
}

def CnotOp : Netqasm_Op<"cnot"> {
  let arguments = (ins VirtualQubitType:$q1, VirtualQubitType:$q2);
}

def CzOp : Netqasm_Op<"cz"> {
  let arguments = (ins VirtualQubitType:$q1, VirtualQubitType:$q2);
}

def CrotXOp : Netqasm_Op<"crot_x"> {
  let arguments = (ins VirtualQubitType:$q1, VirtualQubitType:$q2, I32:$angle_num, I32:$angle_denom);
}

def MeasureOp : Netqasm_Op<"measure"> {
  let arguments = (ins VirtualQubitType:$q);
  let results = (outs I32:$outcome);
}


#endif // NETQASM_OPS_TD