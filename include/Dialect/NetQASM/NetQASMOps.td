#ifndef NETQASM_OPS_TD
#define NETQASM_OPS_TD

include "mlir/IR/OpBase.td"

// Needed for FunctionOpInterface
// Note that NetQASM.h also needs to include "mlir/Interfaces/FunctionInterfaces.h" !
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
// Note that NetQASM.h also needs to include "mlir/Interfaces/SideEffectInterfaces.h" !
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/NetQASM/NetQASMDialect.td"


class NetQASM_Op<string mnemonic, list<Trait> traits = []> :
  Op<NetQASM_Dialect, mnemonic, traits>;

def RemoteOp: NetQASM_Op<"remote", [Symbol]>;

def InitOp : NetQASM_Op<"init"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q);
}

def RotateXOp : NetQASM_Op<"rot_x"> {
  let arguments = (
    ins Arg<I32, "qubit", [MemWrite]>:$q,
    I32:$angle_num,
    I32:$angle_denom
  );
}

def RotateYOp : NetQASM_Op<"rot_y"> {
  let arguments = (
    ins Arg<I32, "qubit", [MemWrite]>:$q,
    I32:$angle_num,
    I32:$angle_denom
  );
}

def RotateZOp : NetQASM_Op<"rot_z"> {
  let arguments = (
    ins Arg<I32, "qubit", [MemWrite]>:$q,
    I32:$angle_num,
    I32:$angle_denom
  );
}

def HadamardOp : NetQASM_Op<"hadamard"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q);
}

def CnotOp : NetQASM_Op<"cnot"> {
  let arguments = (
    ins Arg<I32, "qubit1", [MemWrite]>:$q1,
    Arg<I32, "qubit2", [MemWrite]>:$q2
  );
}

def CzOp : NetQASM_Op<"cz"> {
  let arguments = (
    ins Arg<I32, "qubit1", [MemWrite]>:$q1,
    Arg<I32, "qubit2", [MemWrite]>:$q2
  );
}

def CrotXOp : NetQASM_Op<"crot_x"> {
  let arguments = (ins Arg<I32, "qubit1", [MemWrite]>:$q1, Arg<I32, "qubit2", [MemWrite]>:$q2, I32:$angle_num, I32:$angle_denom);
  let arguments = (
    ins Arg<I32, "qubit1", [MemWrite]>:$q1,
    Arg<I32, "qubit2", [MemWrite]>:$q2,
    I32:$angle_num,
    I32:$angle_denom
  );
}

def MeasureOp : NetQASM_Op<"measure"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q);
  let results = (outs I1:$outcome);
}


def EprsOp: NetQASM_Op<"eprs",
                [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
  let arguments = (
    ins Arg<TensorOf<[I32]>, "qubits", [MemWrite]>:$q,
    I32Attr:$N,
    FlatSymbolRefAttr:$remote
  );
}

def EprsMeasureOp: NetQASM_Op<"eprs_measure",
                [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (
      ins Arg<TensorOf<[I32]>, "qubits", [MemWrite]>:$q,
      I32Attr:$N,
      FlatSymbolRefAttr:$remote
    );
    let results = (outs TensorOf<[I1]>:$outcomes);
}

#endif // NETQASM_OPS_TD