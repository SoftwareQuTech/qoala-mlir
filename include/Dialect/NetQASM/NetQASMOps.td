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
    let assemblyFormat = [{
        operands attr-dict `:` type($q)
    }];
}

def QInitOp: NetQASM_Op<"init"> {
    let arguments = (ins I32:$q);
    let assemblyFormat = [{
        operands attr-dict
    }];
}

def QFreeOp: NetQASM_Op<"qfree"> {
    let arguments = (ins I32:$q);
    let assemblyFormat = [{
        operands attr-dict
    }];
}

/* Helper class ported from QNet dialect */
class UsingRemote_Op<string mnemonic, list<Trait> extraTraits = []> :
    NetQASM_Op<mnemonic, extraTraits # [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
        let extraClassDefinition = [{
            LogicalResult netqasm::$cppClass::verifySymbolUses(SymbolTableCollection &symbolTable) {
                // Check that the callee attribute was specified.
                auto fnAttr = (*this)->getAttrOfType<FlatSymbolRefAttr>("remote");
                if (!fnAttr)
                    return this->emitOpError("requires a 'remote' symbol reference attribute");
                // Search the symbol in the parent SymbolTables
                // The declared symbol MUST come from a qremote.remote operation
                if (symbolTable.lookupNearestSymbolFrom<qremote::RemoteOp>(this->getOperation(), this->getRemoteAttr())) {
                    return success();
                }
                return this->emitOpError() << "'" << fnAttr.getValue()
                                           << "' does not reference a valid remote node";
            }
        }];
}

/* Region verifier function for LocalRoutineOp and RequestRoutineOp */
class ConstraintRegion_Op<string mnemonic, list<Trait> extraTraits = []> :
    NetQASM_Op<mnemonic, extraTraits> {
        let extraClassDefinition = [{
            LogicalResult $cppClass::verifyRegions() {
                for (Operation &operation : this->getBody().getOps()) {
                    if (NetQASMDialect::opIsNotFromAllowedDialects(operation)) {
                        return this->emitError() << "'" << getOperationName() << "' "
                                                 << "op contains an operation that is not from  the allowed list of dialects: ["
                                                 << NetQASMDialect::getAllowedDialectNames()
                                                 << "]. Operation: '" << operation << "'";
                    }
                }
                return success();
            }
        }];
}

def LocalRoutineOp: ConstraintRegion_Op<"local_routine", [
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
        mlir::Region *getCallableRegion() { return isExternal() ? nullptr : &getBody(); }

        /// Returns the argument types of this function.
        mlir::ArrayRef<mlir::Type> getArgumentTypes() { return getFunctionType().getInputs(); }

        /// Returns the result types of this function.
        mlir::ArrayRef<mlir::Type> getResultTypes() { return getFunctionType().getResults(); }

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

def RequestRoutineOp: ConstraintRegion_Op<"request_routine", [
        AffineScope, AutomaticAllocationScope, FunctionOpInterface,
        IsolatedFromAbove, Symbol]> {
    let summary = "Function declaration/definition for a netqasm request routine";
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
        mlir::Region *getCallableRegion() { return isExternal() ? nullptr : &getBody(); }

        /// Returns the argument types of this function.
        mlir::ArrayRef<mlir::Type> getArgumentTypes() { return getFunctionType().getInputs(); }

        /// Returns the result types of this function.
        mlir::ArrayRef<mlir::Type> getResultTypes() { return getFunctionType().getResults(); }

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

def ReturnOp: NetQASM_Op<"return", [Pure, ParentOneOf<["LocalRoutineOp", "RequestRoutineOp"]>, ReturnLike, Terminator]> {
    let summary = "Function return operation for NetQASM call function bodies";
    let arguments = (ins Variadic<AnyType>:$operands);
    let builders = [OpBuilder<(ins), [{
        build($_builder, $_state, std::nullopt);
    }]>];
    let assemblyFormat = [{
        attr-dict ($operands^ `:` type($operands))?
    }];
}

def RotateXOp : NetQASM_Op<"rot_x"> {
    let arguments = (
        ins Arg<I32, "qubit", [MemWrite]>:$q,
        AnyI32Attr:$angle_num,
        AnyI32Attr:$angle_denom
    );
    let assemblyFormat = [{
        operands ` ` `(` $angle_num `,` $angle_denom `)` attr-dict
    }];
}

def RotateYOp : NetQASM_Op<"rot_y"> {
    let arguments = (
        ins Arg<I32, "qubit", [MemWrite]>:$q,
        AnyI32Attr:$angle_num,
        AnyI32Attr:$angle_denom
    );
    let assemblyFormat = [{
        operands ` ` `(` $angle_num `,` $angle_denom `)` attr-dict
    }];
}

def RotateZOp : NetQASM_Op<"rot_z"> {
    let arguments = (
        ins Arg<I32, "qubit", [MemWrite]>:$q,
        AnyI32Attr:$angle_num,
        AnyI32Attr:$angle_denom
    );
    let assemblyFormat = [{
        operands ` ` `(` $angle_num `,` $angle_denom `)` attr-dict
    }];
}

def HadamardOp : NetQASM_Op<"hadamard"> {
    let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q);
    let assemblyFormat = [{
        operands attr-dict
    }];
}

def CnotOp : NetQASM_Op<"cnot"> {
    let arguments = (
        ins Arg<I32, "qubit1", [MemWrite]>:$q1,
        Arg<I32, "qubit2", [MemWrite]>:$q2
    );
    let assemblyFormat = [{
        operands attr-dict
    }];
}

def CzOp : NetQASM_Op<"cz"> {
    let arguments = (
        ins Arg<I32, "qubit1", [MemWrite]>:$q1,
        Arg<I32, "qubit2", [MemWrite]>:$q2
    );
    let assemblyFormat = [{
        operands attr-dict
    }];
}

def CrotXOp : NetQASM_Op<"crot_x"> {
    let arguments = (ins Arg<I32, "qubit1", [MemWrite]>:$q1, Arg<I32, "qubit2", [MemWrite]>:$q2, I32:$angle_num, I32:$angle_denom);
    let arguments = (
        ins Arg<I32, "qubit1", [MemWrite]>:$q1,
        Arg<I32, "qubit2", [MemWrite]>:$q2,
        AnyI32Attr:$angle_num,
        AnyI32Attr:$angle_denom
    );
    let assemblyFormat = [{
        operands ` ` `(` $angle_num `,` $angle_denom `)` attr-dict
    }];
}

def MeasureOp : NetQASM_Op<"measure"> {
    let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q);
    let results = (outs I1:$outcome);
    let assemblyFormat = [{
        operands attr-dict `:` type($outcome)
    }];
}


def EprsOp: UsingRemote_Op<"eprs"> {
    let arguments = (
        ins Arg<I32, "qubits", [MemWrite]>:$q,
        FlatSymbolRefAttr:$remote
    );
    let assemblyFormat = [{
        operands attr-dict
    }];
}

def EprsMeasureOp: UsingRemote_Op<"eprs_measure"> {
    let arguments = (
      ins Arg<I32, "qubits", [MemWrite]>:$q,
      FlatSymbolRefAttr:$remote
    );
    let results = (outs I1:$outcome);
    let assemblyFormat = [{
        operands attr-dict `:` type($outcome)
    }];
}

#endif // NETQASM_OPS_TD