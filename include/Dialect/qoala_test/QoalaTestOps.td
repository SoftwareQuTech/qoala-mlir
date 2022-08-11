#ifndef QOALAtest_OPS_TD
#define QOALAtest_OPS_TD

include "mlir/IR/OpBase.td"

// Needed for FunctionOpInterface
// Note that QoalaTest.h also needs to include "mlir/Interfaces/SideEffectInterfaces.h" !
include "mlir/IR/FunctionInterfaces.td"

include "mlir/IR/SymbolInterfaces.td"
include "mlir/Interfaces/CallInterfaces.td"
include "mlir/Interfaces/CastInterfaces.td"
include "mlir/Interfaces/ControlFlowInterfaces.td"
include "mlir/Interfaces/InferTypeOpInterface.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "mlir/IR/OpAsmInterface.td"

// Needed for `[NoSideEffect]`
// Note that QoalaTest.h also needs to include "mlir/Interfaces/SideEffectInterfaces.h" !
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/qoala_test/QoalaTestDialect.td"


class QoalaTest_Op<string mnemonic, list<Trait> traits = []> :
  Op<QoalaTest_Dialect, mnemonic, traits>;

def ShapeFuncOp : QoalaTest_Op<"func",
    [AffineScope, AutomaticAllocationScope, CallableOpInterface,
     FunctionOpInterface, IsolatedFromAbove, OpAsmOpInterface, Symbol]> {
  let summary = "Shape function";
  let description = [{
    An operation with a name containing a single `SSACFG` region which
    represents a shape transfer function or helper function for shape transfer
    function.
  }];

  let arguments = (ins SymbolNameAttr:$sym_name,
                       TypeAttrOf<FunctionType>:$function_type,
                       OptionalAttr<StrAttr>:$sym_visibility);
  let regions = (region AnyRegion:$body);

  let extraClassDeclaration = [{
    //===------------------------------------------------------------------===//
    // CallableOpInterface
    //===------------------------------------------------------------------===//

    /// Returns the region on the current operation that is callable. This may
    /// return null in the case of an external callable object, e.g. an external
    /// function.
    ::mlir::Region *getCallableRegion() { return isExternal() ? nullptr : &getBody(); }

    /// Returns the results types that the callable region produces when
    /// executed.
    ArrayRef<Type> getCallableResults() { return getFunctionType().getResults(); }

    //===------------------------------------------------------------------===//
    // FunctionOpInterface Methods
    //===------------------------------------------------------------------===//

    /// Returns the argument types of this function.
    ArrayRef<Type> getArgumentTypes() { return getFunctionType().getInputs(); }

    /// Returns the result types of this function.
    ArrayRef<Type> getResultTypes() { return getFunctionType().getResults(); }

    //===------------------------------------------------------------------===//
    // OpAsmOpInterface
    //===------------------------------------------------------------------===//

    // This will filter the `shape.` prefix in front of operations inside the
    // func body.
    static StringRef getDefaultDialect() { return "shape";}

    //===------------------------------------------------------------------===//
    // SymbolOpInterface Methods
    //===------------------------------------------------------------------===//

    bool isDeclaration() { return isExternal(); }
  }];
  let hasCustomAssemblyFormat = 1;
}


#endif // QOALAtest_OPS_TD