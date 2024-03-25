#ifndef QNET_OPS_TD
#define QNET_OPS_TD

include "mlir/IR/OpBase.td"

// Needed for FunctionOpInterface
// Note that Qnet.h also needs to include "mlir/Interfaces/FunctionInterfaces.h" !
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
// Note that Qnet.h also needs to include "mlir/Interfaces/SideEffectInterfaces.h" !
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/Qnet/QnetDialect.td"
include "Dialect/Qnet/QnetTypes.td"


class Qnet_Op<string mnemonic, list<Trait> traits = []> :
  Op<Qnet_Dialect, mnemonic, traits>;


def RemoteOp: Qnet_Op<"remote", [Symbol]>;

// TODO: check if something like this is possible: have a RemoteUsingOp class
// that has the SymbolUserOpInterface trait, like this:
//
// class RemoteUsing_Op<string mnemonic> :
//   Qnet_Op<mnemonic, [DeclareOpInterfaceMethods<SymbolUserOpInterface>]>;
//
// Then, ops that use "remote" symbols can just inherit from this class, like so: 
// 
// def SendIntsOp : RemoteUsing_Op<"send_ints"> {
//     let arguments = (ins TensorOf<[I32]>:$cin);
// }
//
// What's more, this may mean that in `Qnet.cpp` we can just define `verifySymbolUses`
// once, for RemoteUsingOp, instead of copying the same function definition for all
// op classes that use remotes.

def SendIntsOp : Qnet_Op<"send_ints",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins TensorOf<[I32]>:$cin);
}

def RecvIntsOp : Qnet_Op<"recv_ints",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {

    let arguments = (ins FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[I32]>:$cout);
}

def SendFloatsOp : Qnet_Op<"send_floats",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins TensorOf<[F32]>:$cin);
}

def RecvFloatsOp : Qnet_Op<"recv_floats",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {

    let arguments = (ins FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[F32]>:$cout);
}


def NewQubitOp : Qnet_Op<"new_qubit"> {
    let results = (outs QubitType:$qout);
    let assemblyFormat = [{
        attr-dict `:` type($qout)
    }];
}

def RotXOp : Qnet_Op<"rot_x"> {
    let arguments = (ins QubitType:$qin, F32:$angle);
    let results = (outs QubitType:$qout);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout)
    }];
}

def RotYOp : Qnet_Op<"rot_y"> {
    let arguments = (ins QubitType:$qin, F32:$angle);
    let results = (outs QubitType:$qout);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout)
    }];
}

def RotZOp : Qnet_Op<"rot_z"> {
    let arguments = (ins QubitType:$qin, F32:$angle);
    let results = (outs QubitType:$qout);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout)
    }];
}

def HadamardOp : Qnet_Op<"hadamard"> {
    let arguments = (ins QubitType:$qin);
    let results = (outs QubitType:$qout);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout)
    }];
}

def CnotOp : Qnet_Op<"cnot"> {
    let arguments = (ins QubitType:$qin0, QubitType:$qin1);
    let results = (outs QubitType:$qout0, QubitType:$qout1);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout0) `,` type($qout1)
    }];
}

def CzOp : Qnet_Op<"cz"> {
    let arguments = (ins QubitType:$qin0, QubitType:$qin1);
    let results = (outs QubitType:$qout0, QubitType:$qout1);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout0) `,` type($qout1)
    }];
}

def CrotXOp : Qnet_Op<"crot_x"> {
    let arguments = (ins QubitType:$qin0, QubitType:$qin1, F32:$angle);
    let results = (outs QubitType:$qout0, QubitType:$qout1);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout0) `,` type($qout1)
    }];
}

def MeasureOp : Qnet_Op<"measure"> {
    let arguments = (ins QubitType:$qin0);
    let results = (outs I1:$outcome);
    let assemblyFormat = [{
        operands attr-dict `:` type($outcome)
    }];
}

def EprsOp: Qnet_Op<"eprs",
                [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins I32Attr:$N, FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[QubitType]>:$qout);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout)
    }];
}

def EprsMeasureOp: Qnet_Op<"eprs_measure",
                [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins I32Attr:$N, FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[I1]>:$outcomes);
    let assemblyFormat = [{
        operands attr-dict `:` type($outcomes)
    }];
}

#endif // QNET_OPS_TD