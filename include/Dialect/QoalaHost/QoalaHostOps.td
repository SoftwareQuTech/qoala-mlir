#ifndef QOALAHOST_OPS_TD
#define QOALAHOST_OPS_TD

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

include "Dialect/QoalaHost/QoalaHostDialect.td"


class QoalaHost_Op<string mnemonic, list<Trait> traits = []> :
  Op<QoalaHost_Dialect, mnemonic, traits>;


def RemoteOp: QoalaHost_Op<"remote", [Symbol]>;

def SendIntsOp : QoalaHost_Op<"send_ints",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins TensorOf<[I32]>:$cin);
}

def RecvIntsOp : QoalaHost_Op<"recv_ints",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {

    let arguments = (ins FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[I32]>:$cout);
}

def SendFloatsOp : QoalaHost_Op<"send_floats",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins TensorOf<[F32]>:$cin);
}

def RecvFloatsOp : QoalaHost_Op<"recv_floats",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {

    let arguments = (ins FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[F32]>:$cout);
}

def QAllocNOp: QoalaHost_Op<"qalloc"> {
    let results = (outs MemRefOf<[I32]>:$q);
}

def QFreeNOp: QoalaHost_Op<"qfree"> {
    let results = (outs MemRefOf<[I32]>:$q);
}

#endif // QOALAHOST_OPS_TD