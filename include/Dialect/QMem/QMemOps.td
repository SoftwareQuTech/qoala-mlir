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
include "Dialect/QMem/QMemInterface.td"


class QMem_Op<string mnemonic, list<Trait> traits = []> :
    Op<QMem_Dialect, mnemonic, traits # [DeclareOpInterfaceMethods<SimpleCloneInterface>]>;

/* Func and Return operations similar to the ones present un QNet */
def FuncOp: QMem_Op<"func", [
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

def ReturnOp: QMem_Op<"return", [Pure, HasParent<"FuncOp">, ReturnLike, Terminator]> {
    let summary = "Function return operation for qmem.func bodies";
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
    QMem_Op<mnemonic, extraTraits # [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
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


def RemoteOp: QMem_Op<"remote", [Symbol]> {
    let arguments = (ins Arg<SymbolNameAttr, "remote-name">:$sym_name,
                         Arg<OptionalAttr<StrAttr>, "visibility">:$sym_visibility);
    let assemblyFormat = [{
        attr-dict $sym_name
    }];
}

// This operation replaces the "init" for an entangled qubit
// %0 = qmem.alloca : i32
// ;; This op had the "semantics" of entangling the qubit
// qmem.eprs %0
def EprsOp: UsingRemote_Op<"eprs"> {
    let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q, FlatSymbolRefAttr:$remote);
    let assemblyFormat = [{
        operands attr-dict
    }];
}

def EprsMeasureOp: UsingRemote_Op<"eprs_measure"> {
    let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q, FlatSymbolRefAttr:$remote);
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

def QAllocOp: QMem_Op<"qalloc"> {
    let results = (outs I32:$q);
    let assemblyFormat = [{
        attr-dict `:` type($q)
    }];
}

def InitOp : QMem_Op<"init"> {
    let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q);
    let assemblyFormat = [{
        operands attr-dict
    }];
}

def RotateXOp : QMem_Op<"rot_x"> {
    let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q, F32:$angle);
    let assemblyFormat = [{
        operands attr-dict
    }];
}

def RotateYOp : QMem_Op<"rot_y"> {
    let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q, F32:$angle);
    let assemblyFormat = [{
        operands attr-dict
    }];
}

def RotateZOp : QMem_Op<"rot_z"> {
    let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q, F32:$angle);
    let assemblyFormat = [{
        operands attr-dict
    }];
}

def HadamardOp : QMem_Op<"hadamard"> {
    let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q);
    let assemblyFormat = [{
        operands attr-dict
    }];
}

def CnotOp : QMem_Op<"cnot"> {
    let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$qin0, Arg<I32, "qubit", [MemWrite]>:$qin1);
    let assemblyFormat = [{
        operands attr-dict
    }];
}

def CzOp : QMem_Op<"cz"> {
    let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$qin0, Arg<I32, "qubit", [MemWrite]>:$qin1);
    let assemblyFormat = [{
        operands attr-dict
    }];
}

def CrotXOp : QMem_Op<"crot_x"> {
    let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$qin0, Arg<I32, "qubit", [MemWrite]>:$qin1, F32:$angle);
    let assemblyFormat = [{
        operands attr-dict
    }];
}

def MeasureOp : QMem_Op<"measure"> {
    let arguments = (ins Arg<I32, "qubit", [MemWrite]>:$q);
    let results = (outs I1:$outcome);
    let assemblyFormat = [{
        operands attr-dict `:` type($outcome)
    }];
}
#endif // QMEM_OPS_TD