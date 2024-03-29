#ifndef QNET_OPS_TD
#define QNET_OPS_TD

include "mlir/IR/OpBase.td"

// Needed for FunctionOpInterface
// Note that QNet.h also needs to include "mlir/Interfaces/FunctionInterfaces.h" !
include "mlir/Interfaces/FunctionInterfaces.td"

include "mlir/IR/SymbolInterfaces.td"
include "mlir/IR/CommonTypeConstraints.td"
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


class QNet_Op<string mnemonic, list<Trait> traits = []> :
  Op<QNet_Dialect, mnemonic, traits>;


/* QoalaFunc is a "clone" of func.func, with the extension that is also can contain
 * a local symbol table. This is handful so the qoala.remote operation can be
 * places inside the body of a function (and not only a the module level)
 */
def QoalaFunc: QNet_Op<"func", [FunctionOpInterface, SymbolTable]> {
  let summary = "Function declaration/definition for qoala programs.";
  let description = [{
    This function declaration operation differs from func.func operation in
    the sense that it also carries the `SymbolTable` trait. This allows the
    operations contained within the body region con also declare symbols,
    which comes handy when declaring a quantum remote node within the scope
    of a function. This facility allows placing the `qnet.remote` operation
    in the body of a `qnet.func`, which correctly declares the symbol.
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
}

/* The "return" operation had to be ported, since the func.return operation performs a
 * check that it is returning from an operation from func.func....
 * This operation performs the same check, but with qoala.func
 */
def QoalaReturn: QNet_Op<"return", [Pure, HasParent<"QoalaFunc">, ReturnLike, Terminator]> {
  let summary = "Function return operation for qoala.func bodies";
  let arguments = (ins Variadic<AnyType>:$operands);
  let builders = [OpBuilder<(ins), [{
    build($_builder, $_state, std::nullopt);
  }]>];
  let assemblyFormat = "attr-dict ($operands^ `:` type($operands))?";
 }


def RemoteOp: QNet_Op<"remote", [Symbol]> {
  // Operations that declare or define a symbol (i.e., use the "Symbol" trait) *must*
  // declare the two arguments:
  // https://mlir.llvm.org/docs/SymbolsAndSymbolTables/#defining-or-declaring-a-symbol
  let arguments = (ins SymbolNameAttr:$sym_name,
                       OptionalAttr<StrAttr>:$sym_visibility);

  // Additionally, if the operation *defines* a symbol, it needs to correctly
  // define a body for the `bool isDeclaration();` function, returning false when this
  // operation _defines_ a symbol.
  // HOWEVER, since there is a skeleton that always returns false (i.e., this op _always_
  // defines a symbol) in the Symbol class defined in mlir/IR/SymbolInterfaces.td,
  // there is no need to override that definition.
}

// TODO: check if something like this is possible: have a RemoteUsingOp class
// that has the SymbolUserOpInterface trait, like this:
//
// class RemoteUsing_Op<string mnemonic> :
//   QNet_Op<mnemonic, [DeclareOpInterfaceMethods<SymbolUserOpInterface>]>;
//
// Then, ops that use "remote" symbols can just inherit from this class, like so: 
// 
// def SendIntsOp : RemoteUsing_Op<"send_ints"> {
//     let arguments = (ins TensorOf<[I32]>:$cin);
// }
//
// What's more, this may mean that in `QNet.cpp` we can just define `verifySymbolUses`
// once, for RemoteUsingOp, instead of copying the same function definition for all
// op classes that use remotes.

def SendIntsOp : QNet_Op<"send_ints",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins TensorOf<[I32]>:$cin, FlatSymbolRefAttr:$remote);
    let assemblyFormat = [{
        operands attr-dict `:` type($cin)
    }];
}

def RecvIntsOp : QNet_Op<"recv_ints",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {

    let arguments = (ins FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[I32]>:$cout);
    let assemblyFormat = [{
        operands attr-dict `:` type($cout)
    }];
}

def SendFloatsOp : QNet_Op<"send_floats",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins TensorOf<[F32]>:$cin, FlatSymbolRefAttr:$remote);
    let assemblyFormat = [{
        operands attr-dict `:` type($cin)
    }];
}

def RecvFloatsOp : QNet_Op<"recv_floats",
              [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {

    let arguments = (ins FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[F32]>:$cout);
    let assemblyFormat = [{
        operands attr-dict `:` type($cout)
    }];
}


def NewQubitOp : QNet_Op<"new_qubit"> {
    let results = (outs QubitType:$qout);
    let assemblyFormat = [{
        attr-dict `:` type($qout)
    }];
}

def RotXOp : QNet_Op<"rot_x"> {
    let arguments = (ins QubitType:$qin, F32:$angle);
    let results = (outs QubitType:$qout);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout)
    }];
}

def RotYOp : QNet_Op<"rot_y"> {
    let arguments = (ins QubitType:$qin, F32:$angle);
    let results = (outs QubitType:$qout);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout)
    }];
}

def RotZOp : QNet_Op<"rot_z"> {
    let arguments = (ins QubitType:$qin, F32:$angle);
    let results = (outs QubitType:$qout);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout)
    }];
}

def HadamardOp : QNet_Op<"hadamard"> {
    let arguments = (ins QubitType:$qin);
    let results = (outs QubitType:$qout);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout)
    }];
}

def CnotOp : QNet_Op<"cnot"> {
    let arguments = (ins QubitType:$qin0, QubitType:$qin1);
    let results = (outs QubitType:$qout0, QubitType:$qout1);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout0) `,` type($qout1)
    }];
}

def CzOp : QNet_Op<"cz"> {
    let arguments = (ins QubitType:$qin0, QubitType:$qin1);
    let results = (outs QubitType:$qout0, QubitType:$qout1);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout0) `,` type($qout1)
    }];
}

def CrotXOp : QNet_Op<"crot_x"> {
    let arguments = (ins QubitType:$qin0, QubitType:$qin1, F32:$angle);
    let results = (outs QubitType:$qout0, QubitType:$qout1);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout0) `,` type($qout1)
    }];
}

def MeasureOp : QNet_Op<"measure"> {
    let arguments = (ins QubitType:$qin0);
    let results = (outs I1:$outcome);
    let assemblyFormat = [{
        operands attr-dict `:` type($outcome)
    }];
}

def EprsOp: QNet_Op<"eprs",
                [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins I32Attr:$N, FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[QubitType]>:$qout);
    let assemblyFormat = [{
        operands attr-dict `:` type($qout)
    }];
}

def EprsMeasureOp: QNet_Op<"eprs_measure",
                [DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
    let arguments = (ins I32Attr:$N, FlatSymbolRefAttr:$remote);
    let results = (outs TensorOf<[I1]>:$outcomes);
    let assemblyFormat = [{
        operands attr-dict `:` type($outcomes)
    }];
}

#endif // QNET_OPS_TD