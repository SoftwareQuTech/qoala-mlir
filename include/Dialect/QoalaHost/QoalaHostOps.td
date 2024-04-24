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

/* Func and Return operations similar to the ones present un QNet */
def MainFuncOp: QoalaHost_Op<"main_func", [
        AffineScope, AutomaticAllocationScope, FunctionOpInterface,
        IsolatedFromAbove, Symbol]> {
    let summary = "Function wrapper for the operations that are supposed to run on the host machine (CPS)";
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

    let builders = [OpBuilder<(ins
        "::llvm::StringRef":$name, "::mlir::FunctionType":$type,
        CArg<"::llvm::ArrayRef<::mlir::NamedAttribute>", "{}">:$attrs,
        CArg<"::llvm::ArrayRef<::mlir::DictionaryAttr>", "{}">:$argAttrs)
    >];
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
}

def ReturnOp: QoalaHost_Op<"return", [Pure, HasParent<"MainFuncOp">, ReturnLike, Terminator]> {
    let summary = "Function return operation for qoalahost.main_func bodies";
    let arguments = (ins Variadic<AnyType>:$operands);
    let builders = [OpBuilder<(ins), [{
        build($_builder, $_state, std::nullopt);
    }]>];
    let assemblyFormat = [{
        attr-dict ($operands^ `:` type($operands))?
    }];
}

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


def RemoteOp: QoalaHost_Op<"remote", [Symbol]> {
    let arguments = (ins Arg<SymbolNameAttr, "remote-name">:$sym_name,
                         Arg<OptionalAttr<StrAttr>, "visibility">:$sym_visibility);
    let assemblyFormat = [{
        attr-dict $sym_name
    }];
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

#endif // QOALAHOST_OPS_TD