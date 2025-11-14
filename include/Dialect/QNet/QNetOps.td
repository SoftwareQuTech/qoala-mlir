#ifndef QNET_OPS_TD
#define QNET_OPS_TD

include "mlir/IR/OpBase.td"

// Needed for FunctionOpInterface
// Note that QNet.h also needs to include "mlir/Interfaces/FunctionInterfaces.h" !
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
// Note that QNet.h also needs to include "mlir/Interfaces/SideEffectInterfaces.h" !
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/QNet/QNetDialect.td"
include "Dialect/QNet/QNetTypes.td"
include "Analysis/Helpers/QNetInterfaces.td"


class QNet_Op<string mnemonic, list<Trait> traits = []> :
    Op<QNet_Dialect, mnemonic, traits>;

/* This class is defined so that we can implement a "generic" way to implement the 'verifySymbolUses' method
 * It provides an implementation that searches for the symbol specified in the "remote" attribute of the
 * given operation.
 * Any operation that needs to use this implementation of the 'verifySymbolUses', must use this class as the
 * super class, and pass any other traits needed
 */
class UsingRemote_Op<string mnemonic, list<Trait> extraTraits = []> :
    QNet_Op<mnemonic, extraTraits # [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
        let extraClassDefinition = [{
            LogicalResult $cppClass::verifySymbolUses(SymbolTableCollection &symbolTable) {
                // Check that the callee attribute was specified.
                auto fnAttr = (*this)->getAttrOfType<FlatSymbolRefAttr>("remote");
                if (!fnAttr)
                    return this->emitOpError("requires a 'remote' symbol reference attribute");
                // Search the symbol in the parent SymbolTables
                // The declared symbol MUST come from a qnet.remote operation
                if (symbolTable.lookupNearestSymbolFrom<RemoteOp>(this->getOperation(), this->getRemoteAttr())) {
                    return success();
                }
                return this->emitOpError() << "'" << fnAttr.getValue()
                                           << "' does not reference a valid remote node";
            }
        }];
}

/* FuncOp is a "clone" of func.func, with the extension that is also can contain
 * a local symbol table. This is handful so the qoala.remote operation can be
 * places inside the body of a function (and not only a the module level)
 */
def FuncOp: QNet_Op<"func", [
        AffineScope, AutomaticAllocationScope, FunctionOpInterface,
        IsolatedFromAbove, Symbol]> {
    let summary = "Function declaration/definition for qoala programs.";
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
}

/* The "return" operation had to be ported, since the func.return operation performs a
 * check that it is returning from an operation from func.func....
 * This operation performs the same check, but with qnet.func
 */
def ReturnOp: QNet_Op<"return", [Pure, HasParent<"FuncOp">, ReturnLike, Terminator]> {
    let summary = "Function return operation for qmem.func bodies";
    let arguments = (ins Variadic<AnyType>:$operands);
    let builders = [OpBuilder<(ins), [{
        build($_builder, $_state, std::nullopt);
    }]>];
    let assemblyFormat = [{
        attr-dict ($operands^ `:` type($operands))?
    }];
}


def RemoteOp: QNet_Op<"remote", [Symbol]> {
    /* Operations that declare or define a symbol (i.e., use the "Symbol" trait) *must*
    * declare the two arguments:
    * https://mlir.llvm.org/docs/SymbolsAndSymbolTables/#defining-or-declaring-a-symbol
    */
    let arguments = (ins Arg<SymbolNameAttr, "remote-name">:$sym_name,
                         Arg<OptionalAttr<StrAttr>, "visibility">:$sym_visibility);

    /* Additionally, if the operation *defines* a symbol, it needs to correctly
    * define a body for the `bool isDeclaration();` function, returning false when this
    * operation _defines_ a symbol.
    * HOWEVER, since there is a skeleton that always returns false (i.e., this op _always_
    * defines a symbol) in the Symbol class defined in mlir/IR/SymbolInterfaces.td,
    * there is no need to override that definition.
    */
    let assemblyFormat = [{
        attr-dict $sym_name
    }];
}

/* Operations for handling remote nodes/qubits */

def EprsOp: UsingRemote_Op<"eprs", [QubitOpIface, DeclareOpInterfaceMethods<QubitOpIface, ["getQubitOperands"]>]> {
    let arguments = (ins FlatSymbolRefAttr:$remote);
    let results = (outs QubitType:$qout);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout)
    }];
}

def EprsMeasureOp: UsingRemote_Op<"eprs_measure"> {
    let arguments = (ins FlatSymbolRefAttr:$remote);
    let results = (outs I1:$outcome);
    let assemblyFormat = [{
        operands attr-dict `:` type($outcome)
    }];
}

def SendIntsOp : UsingRemote_Op<"send_ints"> {
    let arguments = (ins TensorOf<[I32]>:$cin, FlatSymbolRefAttr:$remote);
    let assemblyFormat = [{
        operands attr-dict `:` type($cin)
    }];
}

def RecvIntsOp : UsingRemote_Op<"recv_ints"> {
    let arguments = (ins FlatSymbolRefAttr:$remote, AnyI32Attr:$length);
    let results = (outs TensorOf<[I32]>:$cout);
    let assemblyFormat = [{
        operands attr-dict `:` type($cout)
    }];
}

def SendFloatsOp : UsingRemote_Op<"send_floats"> {
    let arguments = (ins TensorOf<[F32]>:$cin, FlatSymbolRefAttr:$remote);
    let assemblyFormat = [{
        operands attr-dict `:` type($cin)
    }];
}

def RecvFloatsOp : UsingRemote_Op<"recv_floats"> {
    let arguments = (ins FlatSymbolRefAttr:$remote, AnyI32Attr:$length);
    let results = (outs TensorOf<[F32]>:$cout);
    let assemblyFormat = [{
        operands attr-dict `:` type($cout)
    }];
}

/* Operations for local qubits (can also be applied to an entangled qubit) */

def NewQubitOp : QNet_Op<"new_qubit", [QubitOpIface, DeclareOpInterfaceMethods<QubitOpIface, ["getQubitOperands"]>]> {
    let results = (outs QubitType:$qout);
    let assemblyFormat = [{
        attr-dict `:` type($qout)
    }];
}

def RotXOp : QNet_Op<"rot_x", [QubitOpIface, DeclareOpInterfaceMethods<RotationOpIface>]> {
    let arguments = (ins QubitType:$qin, F32:$angle);
    let results = (outs QubitType:$qout);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout)
    }];
}

def RotYOp : QNet_Op<"rot_y", [QubitOpIface, DeclareOpInterfaceMethods<RotationOpIface>]> {
    let arguments = (ins QubitType:$qin, F32:$angle);
    let results = (outs QubitType:$qout);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout)
    }];
}

def RotZOp : QNet_Op<"rot_z", [QubitOpIface, DeclareOpInterfaceMethods<RotationOpIface>]> {
    let arguments = (ins QubitType:$qin, F32:$angle);
    let results = (outs QubitType:$qout);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout)
    }];
}

def HadamardOp : QNet_Op<"hadamard", [QubitOpIface, HermitianOpIface]> {
    let arguments = (ins QubitType:$qin);
    let results = (outs QubitType:$qout);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout)
    }];
}

def CnotOp : QNet_Op<"cnot", [DeclareOpInterfaceMethods<QubitOpIface, ["isTwoQubitOp"]>, HermitianOpIface]> {
    let arguments = (ins QubitType:$qin0, QubitType:$qin1);
    let results = (outs QubitType:$qout0, QubitType:$qout1);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout0) `,` type($qout1)
    }];
}

def CzOp : QNet_Op<"cz", [DeclareOpInterfaceMethods<QubitOpIface, ["isTwoQubitOp"]>, HermitianOpIface]> {
    let arguments = (ins QubitType:$qin0, QubitType:$qin1);
    let results = (outs QubitType:$qout0, QubitType:$qout1);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout0) `,` type($qout1)
    }];
}

def CrotXOp : QNet_Op<"crot_x", [DeclareOpInterfaceMethods<QubitOpIface, ["isTwoQubitOp"]>, DeclareOpInterfaceMethods<RotationOpIface>]> {
    let arguments = (ins QubitType:$qin0, QubitType:$qin1, F32:$angle);
    let results = (outs QubitType:$qout0, QubitType:$qout1);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout0) `,` type($qout1)
    }];
}

def MeasureOp : QNet_Op<"measure"> {
    let arguments = (ins QubitType:$qin);
    let results = (outs I1:$outcome);
    let assemblyFormat = [{
        operands attr-dict `:` type($outcome)
    }];
}

#endif // QNET_OPS_TD