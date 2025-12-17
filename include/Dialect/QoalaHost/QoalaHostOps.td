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
include "Analysis/Helpers/GenericInterfaces.td"


class QoalaHost_Op<string mnemonic, list<Trait> traits = []> :
    Op<QoalaHost_Dialect, mnemonic, traits # [DeclareOpInterfaceMethods<QuantumOpInterface>]>;

/* Func, Call and Return operations similar to the ones present un QNet */
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
    // We need to define a custom (extra) verifier for checking that the
    // function only contains operations from the `qoalahost`, `arith`, `memref`, `cf` dialects
    let hasRegionVerifier = 1;
}

/* According to the Qoala paper, iQoala QL blocks might only contain
 * calls to local routines. To ease the MLIR analysis, we add the
 * "Terminator" trait for the qoalahost.call operation. This effectively
 * only allows *one single* call instruction per block and must be the last
 * instruction of the block, since "call" will be a terminator operation.
 * This implies that no other call instructions are allowed in the same MLIR
 * block, since a block can only have one terminator.
 * In a more pragmatic way, we will "isolate" call instructions inside its own
 * block, so we can easily comply with the traits of the call operation.
 */
def CallOp : QoalaHost_Op<"call",
     [CallOpInterface, MemRefsNormalizable,
      DeclareOpInterfaceMethods<SymbolUserOpInterface>, Terminator,
      DeclareOpInterfaceMethods<QuantumOpInterface, ["getDuration", "getBlockType"]>]> {
    let summary = "call operation";
    let description = [{
        The `func.call` operation represents a direct call to a function that is
        within the same symbol scope as the call. The operands and result types of
        the call must match the specified function type. The callee is encoded as a
        symbol reference attribute named "callee".
    }];

    let arguments = (ins FlatSymbolRefAttr:$callee, Variadic<AnyType>:$operands);
    let results = (outs Variadic<AnyType>);

    let builders = [
        OpBuilder<(ins "netqasm::LocalRoutineOp":$callee, CArg<"mlir::ValueRange", "{}">:$operands), [{
            $_state.addOperands(operands);
            $_state.addAttribute("callee", mlir::SymbolRefAttr::get(callee));
            $_state.addTypes(callee.getFunctionType().getResults());
        }]>,
        OpBuilder<(ins "netqasm::RequestRoutineOp":$callee, CArg<"mlir::ValueRange", "{}">:$operands), [{
            $_state.addOperands(operands);
            $_state.addAttribute("callee", mlir::SymbolRefAttr::get(callee));
            $_state.addTypes(callee.getFunctionType().getResults());
        }]>,
        OpBuilder<(ins "mlir::SymbolRefAttr":$callee, "mlir::TypeRange":$results,
            CArg<"mlir::ValueRange", "{}">:$operands), [{
                $_state.addOperands(operands);
                $_state.addAttribute("callee", callee);
                $_state.addTypes(results);
        }]>,
        OpBuilder<(ins "mlir::StringAttr":$callee, "mlir::TypeRange":$results,
            CArg<"mlir::ValueRange", "{}">:$operands), [{
                build($_builder, $_state, mlir::SymbolRefAttr::get(callee), results, operands);
        }]>,
        OpBuilder<(ins "::llvm::StringRef":$callee, "mlir::TypeRange":$results,
            CArg<"mlir::ValueRange", "{}">:$operands), [{
                build($_builder, $_state, mlir::StringAttr::get($_builder.getContext(), callee),
                results, operands);
        }]
    >];

    let extraClassDeclaration = [{
        /// Gets the callee operation casted (if possible) to the given type
        template<typename CalleeTy>
        CalleeTy getCalleeOperation() {
            return llvm::dyn_cast_or_null<CalleeTy>(this->getCalleeOperation());
        }

        /// Get the operation of the callee for this call.
        mlir::Operation *getCalleeOperation();

        mlir::FunctionType getCalleeType();

        /// Get the argument operands to the called function.
        operand_range getArgOperands() {
            return {arg_operand_begin(), arg_operand_end()};
        }

        mlir::MutableOperandRange getArgOperandsMutable() {
            return getOperandsMutable();
        }

        operand_iterator arg_operand_begin() { return operand_begin(); }
        operand_iterator arg_operand_end() { return operand_end(); }

        /// Return the callee of this operation.
        mlir::CallInterfaceCallable getCallableForCallee() {
            return (*this)->getAttrOfType<mlir::SymbolRefAttr>("callee");
        }

        /// Set the callee for this operation.
        void setCalleeFromCallable(mlir::CallInterfaceCallable callee) {
            (*this)->setAttr("callee", callee.get<mlir::SymbolRefAttr>());
        }
    }];

    let assemblyFormat = [{
        $callee `(` $operands `)` attr-dict `:` functional-type($operands, results)
    }];

    let hasVerifier = 1;
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

def SendIntOp : UsingRemote_Op<"send_int", [
    DeclareOpInterfaceMethods<QuantumOpInterface, ["getDuration", "getBlockType"]>,
    DeclareOpInterfaceMethods<ClassicalCommInterface>, DeclareOpInterfaceMethods<UsesRemoteInterface>]> {
    let arguments = (ins I32:$cin, FlatSymbolRefAttr:$remote);
    let assemblyFormat = [{
        operands attr-dict `:` type($cin)
    }];
}

/* According to the Qoala paper, iQoala CC blocks might only contain
 * calls to "receive_message". To ease the MLIR analysis, we add the
 * "Terminator" trait for the qoalahost.recv_ints and qoalahost-recv_floats
 * operations. This effectively only allows *one single* recv instruction
 * per block and must be the last instruction of the block, since "recv"
 * will be a terminator operation. This implies that no other recv
 * instructions are allowed in the same MLIR block, since a block can only
 * have one terminator.
 * In a mor pragmatic way, we will "isolate" recv instructions inside its own
 * block, so we can easily comply with the traits of the recv operation.
 */
def RecvIntOp : UsingRemote_Op<"recv_int", [
    Terminator, DeclareOpInterfaceMethods<QuantumOpInterface, ["getDuration", "getBlockType"]>,
    DeclareOpInterfaceMethods<ClassicalCommInterface>, DeclareOpInterfaceMethods<UsesRemoteInterface>]> {
    let arguments = (ins FlatSymbolRefAttr:$remote);
    let results = (outs I32:$cout);
    let assemblyFormat = [{
        operands attr-dict `:` type($cout)
    }];
}

def SendFloatOp : UsingRemote_Op<"send_float", [
    DeclareOpInterfaceMethods<QuantumOpInterface, ["getDuration", "getBlockType"]>,
    DeclareOpInterfaceMethods<ClassicalCommInterface>, DeclareOpInterfaceMethods<UsesRemoteInterface>]> {
    let arguments = (ins F32:$cin, FlatSymbolRefAttr:$remote);
    let assemblyFormat = [{
        operands attr-dict `:` type($cin)
    }];
}

def RecvFloatOp : UsingRemote_Op<"recv_float", [
    Terminator, DeclareOpInterfaceMethods<QuantumOpInterface, ["getDuration", "getBlockType"]>,
    DeclareOpInterfaceMethods<ClassicalCommInterface>, DeclareOpInterfaceMethods<UsesRemoteInterface>]> {
    let arguments = (ins FlatSymbolRefAttr:$remote);
    let results = (outs F32:$cout);
    let assemblyFormat = [{
        operands attr-dict `:` type($cout)
    }];
}

/* "Condensed" (plural) versions of the recv/send_ints/floats
 * These versions are here to prepare for the future
 */
def SendIntsOp : UsingRemote_Op<"send_ints", [
    DeclareOpInterfaceMethods<QuantumOpInterface, ["getDuration", "getBlockType"]>,
    DeclareOpInterfaceMethods<ClassicalCommInterface>, DeclareOpInterfaceMethods<UsesRemoteInterface>]> {
    // TODO - This operation uses a tensor as an input. At this level (LIR)
    //  it should have been lowered to memref. This issue will be addressed by #72
    let arguments = (ins TensorOf<[I32]>:$cin, FlatSymbolRefAttr:$remote);
    let assemblyFormat = [{
        operands attr-dict `:` type($cin)
    }];
}

def RecvIntsOp : UsingRemote_Op<"recv_ints", [
    Terminator, DeclareOpInterfaceMethods<QuantumOpInterface, ["getDuration", "getBlockType"]>,
    DeclareOpInterfaceMethods<ClassicalCommInterface>, DeclareOpInterfaceMethods<UsesRemoteInterface>]> {
    // TODO - This operation uses a tensor as an input. At this level (LIR)
    //  it should have been lowered to memref. This issue will be addressed by #72
    let arguments = (ins FlatSymbolRefAttr:$remote, AnyI32Attr:$length);
    let results = (outs TensorOf<[I32]>:$cout);
    let assemblyFormat = [{
        operands attr-dict `:` type($cout)
    }];
}

def SendFloatsOp : UsingRemote_Op<"send_floats", [
    DeclareOpInterfaceMethods<QuantumOpInterface, ["getDuration", "getBlockType"]>,
    DeclareOpInterfaceMethods<ClassicalCommInterface>, DeclareOpInterfaceMethods<UsesRemoteInterface>]> {
    // TODO - This operation uses a tensor as an input. At this level (LIR)
    //  it should have been lowered to memref. This issue will be addressed by #72
    let arguments = (ins TensorOf<[F32]>:$cin, FlatSymbolRefAttr:$remote);
    let assemblyFormat = [{
        operands attr-dict `:` type($cin)
    }];
}

def RecvFloatsOp : UsingRemote_Op<"recv_floats", [
    Terminator, DeclareOpInterfaceMethods<QuantumOpInterface, ["getDuration", "getBlockType"]>,
    DeclareOpInterfaceMethods<ClassicalCommInterface>, DeclareOpInterfaceMethods<UsesRemoteInterface>]> {
    // TODO - This operation uses a tensor as an input. At this level (LIR)
    //  it should have been lowered to memref. This issue will be addressed by #72
    let arguments = (ins FlatSymbolRefAttr:$remote, AnyI32Attr:$length);
    let results = (outs TensorOf<[F32]>:$cout);
    let assemblyFormat = [{
        operands attr-dict `:` type($cout)
    }];
}

def NopTOp : QoalaHost_Op<"nop_term", [HasParent<"MainFuncOp">, Terminator]> {
    let summary = [{
        "nop" operation with terminator trait. This operation is useful to
        correctly group other operations in their respective blocks, e.g.
        to form "CL" blocks that can only contain operations modelling
        classical, local operations, involving only the CPS.
        Without this nop "terminal" operation, we might need to add
        another terminal operator (like return, branch or even qoalahost.call/recv)
        to not violate the restriction that a block MUST have a terminal
        operation.
    }];
    let assemblyFormat = [{
        operands attr-dict
    }];
}

def NopOp : QoalaHost_Op<"nop", [
    HasParent<"MainFuncOp">, DeclareOpInterfaceMethods<QuantumOpInterface, ["getDuration"]>]> {
    let summary = [{
        QoalaHost "nop" operation. The difference with the NopTOp is that
        this operation does not carry the "Terminator" trait, so it can be
        placed in the middle of a block.
    }];
    let assemblyFormat = [{
        operands attr-dict
    }];
}

def BlkMeta : QoalaHost_Op<"blk_meta", [HasParent<"MainFuncOp">]> {
  let arguments = (ins
    StrAttr:$block_id,
    StrArrayAttr:$predecessors,
    StrArrayAttr: $dependencies,
    StrAttr: $prev_comm,
    StrAttr: $prev_ent,
    DictionaryAttr: $deadlines
  );

  let summary = [{
    A "no-op" operation that semantically does nothing, but is used to store
    metadata about the block in which it appears. Specifically, it records the
    block's unique identifier as a string and a list of its precedences block
    identifiers. This information can be used for analysis or transformations
    without affecting program semantics.
  }];

  let assemblyFormat = [{
    operands attr-dict
  }];
}

#endif // QOALAHOST_OPS_TD