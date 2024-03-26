#ifndef QMEM_OPS_TD
#define QMEM_OPS_TD

include "mlir/IR/OpBase.td"

// Needed for FunctionOpInterface
// Note that Qmem.h also needs to include "mlir/Interfaces/FunctionInterfaces.h" !
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
// Note that Qmem.h also needs to include "mlir/Interfaces/SideEffectInterfaces.h" !
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/Qmem/QmemDialect.td"


class Qmem_Op<string mnemonic, list<Trait> traits = []> :
  Op<Qmem_Dialect, mnemonic, traits>;


def RemoteOp: Qmem_Op<"remote", [Symbol]>;

def SendIntsOp : Qmem_Op<"send_ints",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins TensorOf<[I32]>:$cin);
}

def RecvIntsOp : Qmem_Op<"recv_ints",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {

    let arguments = (ins FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[I32]>:$cout);
}

def SendFloatsOp : Qmem_Op<"send_floats",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins TensorOf<[F32]>:$cin);
}

def RecvFloatsOp : Qmem_Op<"recv_floats",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {

    let arguments = (ins FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[F32]>:$cout);
}

def QAllocOp: Qmem_Op<"qalloc"> {
    let results = (outs I32:$q);
}

def QAllocNOp: Qmem_Op<"qalloc_n"> {
    let results = (outs MemRefOf<[I32]>:$q);
}

def InitOp : Qmem_Op<"init"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q);
}

def RotateXOp : Qmem_Op<"rot_x"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q, F32:$angle);
}

def RotateYOp : Qmem_Op<"rot_y"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q, F32:$angle);
}

def RotateZOp : Qmem_Op<"rot_z"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q, F32:$angle);
}

def HadamardOp : Qmem_Op<"hadamard"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q);
}

def CnotOp : Qmem_Op<"cnot"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q1, Arg<I32, "qubit", [MemWrite]>:$q2);
}

def CzOp : Qmem_Op<"cz"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q1, Arg<I32, "qubit", [MemWrite]>:$q2);
}

def CrotXOp : Qmem_Op<"crot_x"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q1, Arg<I32, "qubit", [MemWrite]>:$q2, F32:$angle);
}

def MeasureOp : Qmem_Op<"measure"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q);
  let results = (outs I1:$outcome);
}

def EprsOp: Qmem_Op<"eprs",
                [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
  let arguments = (ins Arg<MemRefOf<[I32]>, "qubits", [MemWrite]>:$q, I32Attr:$N, FlatSymbolRefAttr:$remote);
}

def EprsMeasureOp: Qmem_Op<"eprs_measure",
                [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins Arg<MemRefOf<[I32]>, "qubits", [MemWrite]>:$q, I32Attr:$N, FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[I1]>:$outcomes);
}


#endif // QMEM_OPS_TD