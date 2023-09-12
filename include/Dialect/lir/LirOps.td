#ifndef LIR_OPS_TD
#define LIR_OPS_TD

include "mlir/IR/OpBase.td"

// Needed for FunctionOpInterface
// Note that Lir.h also needs to include "mlir/Interfaces/FunctionInterfaces.h" !
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
// Note that Lir.h also needs to include "mlir/Interfaces/SideEffectInterfaces.h" !
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/lir/LirDialect.td"
include "Dialect/lir/LirTypes.td"


class Lir_Op<string mnemonic, list<Trait> traits = []> :
  Op<Lir_Dialect, mnemonic, traits>;

class Lir_QuantumOp<string mnemonic, list<Trait> traits = []> :
  Lir_Op<mnemonic, traits>;

class Lir_ClassicalOp<string mnemonic, list<Trait> traits = []> :
  Lir_Op<mnemonic, traits>;

def RemoteNodeOp: Lir_Op<"remote_node", [Symbol]>;

// `IsolatedFromAbove` is needed for running an OperationPass on it.
def SubroutineOp : Lir_Op<"subroutine", [FunctionOpInterface, IsolatedFromAbove]> {

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

  let hasCustomAssemblyFormat = 1;
}


def ReturnOp : Lir_Op<"return", [NoMemoryEffect, HasParent<"SubroutineOp">,
                                 Terminator]> {
  // The return operation takes an optional input operand to return. This
  // value must match the return type of the enclosing function.
  // let arguments = (ins Variadic<I32>:$classical, Variadic<QubitType>:$quantum);
  // let arguments = (ins I32:$classical, QubitType:$quantum);
  let arguments = (ins Variadic<AnyType>:$operands);

  let assemblyFormat = "attr-dict ($operands^ `:` type($operands))?";
}

def CallOp : Lir_Op<"call", [CallOpInterface, MemRefsNormalizable,
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

  let assemblyFormat = [{
    $callee `(` $operands `)` attr-dict `:` functional-type($operands, results)
  }];
}


def ConstantOp : Lir_Op<"constant"> {
    let arguments = (ins I32Attr:$value);

    let results = (outs I32);
}


def NewQubitOp : Lir_QuantumOp<"new_qubit"> {
    let results = (outs QubitType:$qout);
}

def NewCommQubitOp : Lir_QuantumOp<"new_comm_qubit"> {
    let results = (outs CommQubitType:$qout);
}

def EntangleKeepWithFailOp : Lir_QuantumOp<"entangle_keep_with_fail"> {
    let results = (outs EntQubitType:$qout, I1:$success);
}

def EntangleKeepOp: Lir_QuantumOp<"entangle_keep",
                [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins FlatSymbolRefAttr:$remote_node);
    let results = (outs QubitType:$qout);

  let assemblyFormat = [{
    $remote_node attr-dict
  }];
}

def EntangleMeasureOp : Lir_QuantumOp<"entangle_measure"> {
    let results = (outs QubitType:$qout);
}

def EntToLocal : Lir_QuantumOp<"ent_to_local"> {
  let arguments = (ins EntQubitType:$qin);
  let results = (outs QubitType:$qout);
}

def SendMsgOp : Lir_ClassicalOp<"send_msg",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins I32:$cin);
}

def ReceiveMsgOp : Lir_ClassicalOp<"receive_msg",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {

    let arguments = (ins FlatSymbolRefAttr:$remote_node);
    let results = (outs I32:$cout);

  let assemblyFormat = [{
    $remote_node attr-dict `:` type($cout)
  }];
}

def AngleGateOp : Lir_QuantumOp<"angle_gate"> {
  let arguments = (ins QubitType:$qin, F32:$cin);
  let results = (outs QubitType:$qout);
}

def RotateZOp : Lir_QuantumOp<"rot_z"> {
  let arguments = (ins QubitType:$qin, I32:$cin);
  let results = (outs QubitType:$qout);

  let assemblyFormat = [{
    $qin `,` $cin attr-dict
  }];
}

def GateOp : Lir_QuantumOp<"gate"> {
  let arguments = (ins QubitType:$qin);
  let results = (outs QubitType:$qout);
}

def HadamardOp : Lir_QuantumOp<"had"> {
  let arguments = (ins QubitType:$qin);
  let results = (outs QubitType:$qout);

  let assemblyFormat = [{
    $qin attr-dict
  }];
}

def TwoQubitGateOp : Lir_QuantumOp<"two_qubit_gate"> {
  let arguments = (ins QubitType:$qin0, QubitType:$qin1);
  let results = (outs QubitType:$qout0, QubitType:$qout1);
}

def CphaseOp : Lir_QuantumOp<"cphase"> {
  let arguments = (ins QubitType:$qin0, QubitType:$qin1);
  let results = (outs QubitType:$qout0, QubitType:$qout1);
}

def MeasureOp : Lir_QuantumOp<"measure"> {
  let arguments = (ins QubitType:$qin);
  let results = (outs I1:$cout);

  let assemblyFormat = [{
    $qin attr-dict
  }];
}

def AddOp : Lir_ClassicalOp<"add"> {
  let arguments = (ins I32:$cin0, I32:$cin1);
  let results = (outs I32:$cout);
}


#endif // LIR_OPS_TD