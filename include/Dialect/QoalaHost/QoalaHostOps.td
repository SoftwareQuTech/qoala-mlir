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

/* Helper class ported from QNet dialect */
class UsingRemote_Op<string mnemonic, list<Trait> extraTraits = []> :
    QoalaHost_Op<mnemonic, extraTraits # [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
        let extraClassDefinition = [{
            LogicalResult $cppClass::verifySymbolUses(SymbolTableCollection &symbolTable) {
                // Check that the callee attribute was specified.
                auto fnAttr = (*this)->getAttrOfType<FlatSymbolRefAttr>("remote");
                if (!fnAttr)
                    return this->emitOpError("requires a 'remote' symbol reference attribute");
                // Iteratively search on the parent SymbolTables
                if (auto nearest = symbolTable.lookupNearestSymbolFrom<RemoteOp>(this->getOperation(), this->getRemoteAttr())) {
                    return success();
                }
                return this->emitOpError() << "'" << fnAttr.getValue()
                                           << "' does not reference a valid remote node";
            }
        }];
}


def RemoteOp: QoalaHost_Op<"remote", [Symbol]>;

def SendIntsOp : UsingRemote_Op<"send_ints"> {
    let arguments = (ins TensorOf<[I32]>:$cin, FlatSymbolRefAttr:$remote);
}

def RecvIntsOp : UsingRemote_Op<"recv_ints"> {
    let arguments = (ins FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[I32]>:$cout);
}

def SendFloatsOp : UsingRemote_Op<"send_floats"> {
    let arguments = (ins TensorOf<[F32]>:$cin, FlatSymbolRefAttr:$remote);
}

def RecvFloatsOp : UsingRemote_Op<"recv_floats"> {
    let arguments = (ins FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[F32]>:$cout);
}

def QAllocOp: QoalaHost_Op<"qalloc"> {
    let results = (outs TensorOf<[I32]>:$q);
}

def QFreeOp: QoalaHost_Op<"qfree"> {
    let results = (outs TensorOf<[I32]>:$q);
}

#endif // QOALAHOST_OPS_TD