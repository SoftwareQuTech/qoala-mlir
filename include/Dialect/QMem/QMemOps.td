#ifndef QMEM_OPS_TD
#define QMEM_OPS_TD

include "mlir/IR/OpBase.td"

// Needed for FunctionOpInterface
// Note that QMem.h also needs to include "mlir/Interfaces/FunctionInterfaces.h" !
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
// Note that QMem.h also needs to include "mlir/Interfaces/SideEffectInterfaces.h" !
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/QMem/QMemDialect.td"


class QMem_Op<string mnemonic, list<Trait> traits = []> :
  Op<QMem_Dialect, mnemonic, traits>;


def RemoteOp: QMem_Op<"remote", [Symbol]>;

def SendIntsOp : QMem_Op<"send_ints",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins TensorOf<[I32]>:$cin);
}

def RecvIntsOp : QMem_Op<"recv_ints",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {

    let arguments = (ins FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[I32]>:$cout);
}

def SendFloatsOp : QMem_Op<"send_floats",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins TensorOf<[F32]>:$cin);
}

def RecvFloatsOp : QMem_Op<"recv_floats",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {

    let arguments = (ins FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[F32]>:$cout);
}

def QAllocOp: QMem_Op<"qalloc"> {
    let results = (outs I32:$q);
}

def InitOp : QMem_Op<"init"> {
  let arguments = (ins I32:$q);
}

def RotateXOp : QMem_Op<"rot_x"> {
  let arguments = (ins I32:$q, F32:$angle);
}

def RotateYOp : QMem_Op<"rot_y"> {
  let arguments = (ins I32:$q, F32:$angle);
}

def RotateZOp : QMem_Op<"rot_z"> {
  let arguments = (ins I32:$q, F32:$angle);
}

def HadamardOp : QMem_Op<"hadamard"> {
  let arguments = (ins I32:$q);
}

def CnotOp : QMem_Op<"cnot"> {
  let arguments = (ins I32:$q1, I32:$q2);
}

def CzOp : QMem_Op<"cz"> {
  let arguments = (ins I32:$q1, I32:$q2);
}

def CrotXOp : QMem_Op<"crot_x"> {
  let arguments = (ins I32:$q1, I32:$q2, F32:$angle);
}

def MeasureOp : QMem_Op<"measure"> {
  let arguments = (ins I32:$q);
  let results = (outs I1:$outcome);
}

def EprsOp: QMem_Op<"eprs",
                [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
  let arguments = (ins MemRefOf<[I32]>:$q, I32Attr:$N, FlatSymbolRefAttr:$remote);
}

def EprsMeasureOp: QMem_Op<"eprs_measure",
                [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins MemRefOf<[I32]>:$q, I32Attr:$N, FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[I1]>:$outcomes);
}


#endif // QMEM_OPS_TD