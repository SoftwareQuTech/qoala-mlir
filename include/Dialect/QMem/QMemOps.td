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

/* Helper class ported from QNet dialect */
class UsingRemote_Op<string mnemonic, list<Trait> extraTraits = []> :
  QMem_Op<mnemonic, extraTraits # [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
  let extraClassDefinition = [{
    LogicalResult $cppClass::verifySymbolUses(SymbolTableCollection &symbolTable) {
        // Check that the callee attribute was specified.
        auto fnAttr = (*this)->getAttrOfType<FlatSymbolRefAttr>("remote");
        if (!fnAttr)
            return this->emitOpError("requires a 'remote' symbol reference attribute");
        // Iteratively search on the parent SymbolTables
        for (auto *parent = this->getOperation()->getParentOp(); parent; parent = parent->getParentOp()) {
            if (auto nearest = symbolTable.lookupNearestSymbolFrom<RemoteOp>(parent, this->getRemoteAttr())) {
                return success();
            }
        }
        return this->emitOpError() << "'" << fnAttr.getValue()
                                   << "' does not reference a valid remote node";
      }
  }];
}


def RemoteOp: QMem_Op<"remote", [Symbol]> {
  let arguments = (ins SymbolNameAttr:$sym_name,
                       OptionalAttr<StrAttr>:$sym_visibility);
}

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

def QAllocOp: QMem_Op<"qalloc"> {
    let results = (outs TensorOf<[I32]>:$q);
}

def InitOp : QMem_Op<"init"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q);
}

def RotateXOp : QMem_Op<"rot_x"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q, F32:$angle);
}

def RotateYOp : QMem_Op<"rot_y"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q, F32:$angle);
}

def RotateZOp : QMem_Op<"rot_z"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q, F32:$angle);
}

def HadamardOp : QMem_Op<"hadamard"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q);
}

def CnotOp : QMem_Op<"cnot"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q1, Arg<I32, "qubit", [MemWrite]>:$q2);
}

def CzOp : QMem_Op<"cz"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q1, Arg<I32, "qubit", [MemWrite]>:$q2);
}

def CrotXOp : QMem_Op<"crot_x"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q1, Arg<I32, "qubit", [MemWrite]>:$q2, F32:$angle);
}

def MeasureOp : QMem_Op<"measure"> {
  let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q);
  let results = (outs I1:$outcome);
}

def EprsOp: UsingRemote_Op<"eprs"> {
  let arguments = (ins Arg<TensorOf<[I32]>, "qubits", [MemWrite]>:$q, I32Attr:$N, FlatSymbolRefAttr:$remote);
}

def EprsMeasureOp: UsingRemote_Op<"eprs_measure"> {
    let arguments = (ins Arg<TensorOf<[I32]>, "qubits", [MemWrite]>:$q, I32Attr:$N, FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[I1]>:$outcomes);
}


#endif // QMEM_OPS_TD