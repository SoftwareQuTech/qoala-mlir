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

def QAllocOp: NetQASM_Op<"qalloc"> {
    let results = (outs I32:$q);
}

def QInitOp: NetQASM_Op<"init"> {
    let arguments = (ins I32:$q);
}

def QFreeOp: NetQASM_Op<"qfree"> {
    let arguments = (ins I32:$q);
}

/* Helper class ported from QNet dialect */
class UsingRemote_Op<string mnemonic, list<Trait> extraTraits = []> :
    NetQASM_Op<mnemonic, extraTraits # [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
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

def LocalRoutineOp: NetQASM_Op<"local_routine", [
        AffineScope, AutomaticAllocationScope, FunctionOpInterface,
        IsolatedFromAbove, Symbol]> {
    let summary = "Function declaration/definition for a netqasm local routine";
    let description = [{
        This function declaration operation differs from func.func operation in
        the sense that it supports function attributes that are not declared
        in the func.func operation.
    }];
    let arguments = (ins SymbolNameAttr:$sym_name,
                         TypeAttrOf<FunctionType>:$function_type,
                         OptionalAttr<StrAttr>:$sym_visibility,
                         OptionalAttr<DictArrayAttr>:$arg_attrs,
                         OptionalAttr<DictArrayAttr>:$res_attrs);
    let regions = (region AnyRegion:$body);
    let extraClassDeclaration = [{
        //===------------------------------------------------------------------===//
        // FunctionOpInterface Methods
        //===------------------------------------------------------------------===//

        /// Returns the region on the current operation that is callable. This may
        /// return null in the case of an external callable object, e.g. an external
        /// function.
        ::mlir::Region *getCallableRegion() { return isExternal() ? nullptr : &getBody(); }

        /// Returns the argument types of this function.
        ::mlir::ArrayRef<::mlir::Type> getArgumentTypes() { return getFunctionType().getInputs(); }

        /// Returns the result types of this function.
        ::mlir::ArrayRef<::mlir::Type> getResultTypes() { return getFunctionType().getResults(); }

        //===------------------------------------------------------------------===//
        // SymbolOpInterface Methods - Inherited form FunctionOpInterface
        //===------------------------------------------------------------------===//

        bool isDeclaration() { return isExternal(); }
    }];
    let hasCustomAssemblyFormat = 1;
    // We need to define a custom (extra) verifier for checking that the
    // function only contains operations from the `arith`, `memref`, `cf` dialects
    let hasRegionVerifier = 1;
}

def RequestRoutineOp: NetQASM_Op<"request_routine", [
        AffineScope, AutomaticAllocationScope, FunctionOpInterface,
        IsolatedFromAbove, Symbol]> {
    let summary = "Function declaration/definition for a netqasm local routine";
    let description = [{
        This function declaration operation differs from func.func operation in
        the sense that it supports function attributes that are not declared
        in the func.func operation.
    }];
    let arguments = (ins SymbolNameAttr:$sym_name,
                         TypeAttrOf<FunctionType>:$function_type,
                         OptionalAttr<StrAttr>:$sym_visibility,
                         OptionalAttr<DictArrayAttr>:$arg_attrs,
                         OptionalAttr<DictArrayAttr>:$res_attrs);
    let regions = (region AnyRegion:$body);
    let extraClassDeclaration = [{
        //===------------------------------------------------------------------===//
        // FunctionOpInterface Methods
        //===------------------------------------------------------------------===//

        /// Returns the region on the current operation that is callable. This may
        /// return null in the case of an external callable object, e.g. an external
        /// function.
        ::mlir::Region *getCallableRegion() { return isExternal() ? nullptr : &getBody(); }

        /// Returns the argument types of this function.
        ::mlir::ArrayRef<::mlir::Type> getArgumentTypes() { return getFunctionType().getInputs(); }

        /// Returns the result types of this function.
        ::mlir::ArrayRef<::mlir::Type> getResultTypes() { return getFunctionType().getResults(); }

        //===------------------------------------------------------------------===//
        // SymbolOpInterface Methods - Inherited form FunctionOpInterface
        //===------------------------------------------------------------------===//

        bool isDeclaration() { return isExternal(); }
    }];
    let hasCustomAssemblyFormat = 1;
    // We need to define a custom (extra) verifier for checking that the
    // function only contains operations from the `arith`, `memref`, `cf` dialects
    let hasRegionVerifier = 1;
}

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


def EprsOp: UsingRemote_Op<"eprs"> {
  let arguments = (
    ins Arg<TensorOf<[I32]>, "qubits", [MemWrite]>:$q,
    I32Attr:$N,
    FlatSymbolRefAttr:$remote
  );
}

def EprsMeasureOp: UsingRemote_Op<"eprs_measure"> {
    let arguments = (
      ins Arg<TensorOf<[I32]>, "qubits", [MemWrite]>:$q,
      I32Attr:$N,
      FlatSymbolRefAttr:$remote
    );
    let results = (outs TensorOf<[I1]>:$outcomes);
}

#endif // NETQASM_OPS_TD