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

include "Dialect/netqasm/NetqasmDialect.td"
include "Dialect/netqasm/NetqasmTypes.td"


class Netqasm_Op<string mnemonic, list<Trait> traits = []> :
  Op<Netqasm_Dialect, mnemonic, traits>;

class Netqasm_QuantumOp<string mnemonic, list<Trait> traits = []> :
  Netqasm_Op<mnemonic, traits>;

class Netqasm_ClassicalOp<string mnemonic, list<Trait> traits = []> :
  Netqasm_Op<mnemonic, traits>;

def RemoteNodeOp: Netqasm_Op<"remote_node", [Symbol]>;

// `IsolatedFromAbove` is needed for running an OperationPass on it.
def SubroutineOp : Netqasm_Op<"subroutine", [FunctionOpInterface, IsolatedFromAbove]> {

  // part with "TypeAttrOf" is needed for compiling lib!
  let arguments = (ins SymbolNameAttr:$sym_name,
    TypeAttrOf<FunctionType>:$function_type,
    OptionalAttr<DictArrayAttr>:$arg_attrs,
    OptionalAttr<DictArrayAttr>:$res_attrs);

  let regions = (region AnyRegion:$body);

  // Needed by the generated (in MLIR itself) FunctionOpInstances.h.inc
  let extraClassDeclaration = [{
    //===------------------------------------------------------------------===//
    // FunctionOpInterface Methods
    //===------------------------------------------------------------------===//

    /// Returns the argument types of this function.
    ArrayRef<Type> getArgumentTypes() { return getFunctionType().getInputs(); }

    /// Returns the result types of this function.
    ArrayRef<Type> getResultTypes() { return getFunctionType().getResults(); }
  
    /// Returns the region on the current operation that is callable. This may
    /// return null in the case of an external callable object, e.g. an external
    /// function.
    ::mlir::Region *getCallableRegion() { return isExternal() ? nullptr : &getBody(); }
  }];
}


def ReturnOp : Netqasm_Op<"return", [NoMemoryEffect, HasParent<"SubroutineOp">,
                                 Terminator]> {
  // The return operation takes an optional input operand to return. This
  // value must match the return type of the enclosing function.
  // let arguments = (ins Variadic<I32>:$classical, Variadic<QubitType>:$quantum);
  let arguments = (ins I32:$classical, CommQubitType:$quantum);
}

def CallOp : Netqasm_Op<"call", [CallOpInterface, MemRefsNormalizable,
     DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
  // The return operation takes an optional input operand to return. This
  // value must match the return type of the enclosing function.
  // let arguments = (ins Variadic<I32>:$classical, Variadic<QubitType>:$quantum);
  let arguments = (ins FlatSymbolRefAttr:$callee, Variadic<AnyType>:$operands);
  let results = (outs Variadic<AnyType>);

  let extraClassDeclaration = [{
    FunctionType getCalleeType();

    /// Get the argument operands to the called function.
    operand_range getArgOperands() {
      return {arg_operand_begin(), arg_operand_end()};
    }

    MutableOperandRange getArgOperandsMutable() {
      return getOperandsMutable();
    }

    operand_iterator arg_operand_begin() { return operand_begin(); }
    operand_iterator arg_operand_end() { return operand_end(); }

    /// Return the callee of this operation.
    CallInterfaceCallable getCallableForCallee() {
      return (*this)->getAttrOfType<SymbolRefAttr>("callee");
    }

    /// Set the callee for this operation.
    void setCalleeFromCallable(CallInterfaceCallable callee) {
      (*this)->setAttr("callee", callee.get<SymbolRefAttr>());
    }
  }];
}


def ConstantOp : Netqasm_Op<"constant"> {
    let arguments = (ins I32Attr:$value);

    let results = (outs I32);
}


def NewQubitOp : Netqasm_QuantumOp<"new_qubit"> {
    let results = (outs QubitType:$qout);
}

def NewCommQubitOp : Netqasm_QuantumOp<"new_comm_qubit"> {
    let results = (outs CommQubitType:$qout);
}

def EntangleKeepWithFailOp : Netqasm_QuantumOp<"entangle_keep_with_fail"> {
    let results = (outs EntQubitType:$qout, I1:$success);
}

def EntangleKeepOp: Netqasm_QuantumOp<"entangle_keep",
                [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins FlatSymbolRefAttr:$remote_node);
    let results = (outs EntQubitType:$qout);
}

def EntangleMeasureOp : Netqasm_QuantumOp<"entangle_measure"> {
    let results = (outs QubitType:$qout);
}

def EntToLocal : Netqasm_QuantumOp<"ent_to_local"> {
  let arguments = (ins EntQubitType:$qin);
  let results = (outs QubitType:$qout);
}

def SendMsgOp : Netqasm_ClassicalOp<"send_msg",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins I32:$cin);
}

def ReceiveMsgOp : Netqasm_ClassicalOp<"receive_msg",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let results = (outs I32:$cout);
}

def AngleGateOp : Netqasm_QuantumOp<"angle_gate"> {
  let arguments = (ins QubitType:$qin, F32:$cin);
  let results = (outs QubitType:$qout);
}

def RotateZOp : Netqasm_QuantumOp<"rot_z"> {
  let arguments = (ins QubitType:$qin, I32:$cin);
  let results = (outs QubitType:$qout);
}

def GateOp : Netqasm_QuantumOp<"gate"> {
  let arguments = (ins QubitType:$qin);
  let results = (outs QubitType:$qout);
}

def HadamardOp : Netqasm_QuantumOp<"had"> {
  let arguments = (ins QubitType:$qin);
  let results = (outs QubitType:$qout);
}

def TwoQubitGateOp : Netqasm_QuantumOp<"two_qubit_gate"> {
  let arguments = (ins QubitType:$qin0, QubitType:$qin1);
  let results = (outs QubitType:$qout0, QubitType:$qout1);
}

def CphaseOp : Netqasm_QuantumOp<"cphase"> {
  let arguments = (ins QubitType:$qin0, QubitType:$qin1);
  let results = (outs QubitType:$qout0, QubitType:$qout1);
}

def MeasureOp : Netqasm_QuantumOp<"measure"> {
  let arguments = (ins QubitType:$qin);
  let results = (outs I1:$cout);
}

def AddOp : Netqasm_ClassicalOp<"add"> {
  let arguments = (ins I32:$cin0, I32:$cin1);
  let results = (outs I32:$cout);
}


#endif // NETQASM_OPS_TD