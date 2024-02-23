#ifndef HIR_OPS_TD
#define HIR_OPS_TD

include "mlir/IR/OpBase.td"

// Needed for FunctionOpInterface
// Note that Hir.h also needs to include "mlir/Interfaces/FunctionInterfaces.h" !
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
// Note that Hir.h also needs to include "mlir/Interfaces/SideEffectInterfaces.h" !
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/hir/HirDialect.td"
include "Dialect/hir/HirTypes.td"


class Hir_Op<string mnemonic, list<Trait> traits = []> :
  Op<Hir_Dialect, mnemonic, traits>;


def RemoteOp: Hir_Op<"remote", [Symbol]>;

// TODO: check if something like this is possible: have a RemoteUsingOp class
// that has the SymbolUserOpInterface trait, like this:
//
// class RemoteUsing_Op<string mnemonic> :
//   Hir_Op<mnemonic, [DeclareOpInterfaceMethods<SymbolUserOpInterface>]>;
//
// Then, ops that use "remote" symbols can just inherit from this class, like so: 
// 
// def SendIntsOp : RemoteUsing_Op<"send_ints"> {
//     let arguments = (ins TensorOf<[I32]>:$cin);
// }
//
// What's more, this may mean that in `Hir.cpp` we can just define `verifySymbolUses`
// once, for RemoteUsingOp, instead of copying the same function definition for all
// op classes that use remotes.

def SendIntsOp : Hir_Op<"send_ints",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins TensorOf<[I32]>:$cin);
}

def RecvIntsOp : Hir_Op<"recv_ints",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {

    let arguments = (ins FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[I32]>:$cout);

  // let assemblyFormat = [{
  //   $remote_node attr-dict `:` type($cout)
  // }];
}

def SendFloatsOp : Hir_Op<"send_floats",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins TensorOf<[F32]>:$cin);
}

def RecvFloatsOp : Hir_Op<"recv_floats",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {

    let arguments = (ins FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[F32]>:$cout);
}

#endif // HIR_OPS_TD