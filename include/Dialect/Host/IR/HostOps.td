#ifndef HOST_OPS_TD
#define HOST_OPS_TD

include "mlir/IR/OpBase.td"

// Needed for FunctionOpInterface
// Note that Host.h also needs to include "mlir/Interfaces/SideEffectInterfaces.h" !
include "mlir/IR/FunctionInterfaces.td"

include "mlir/IR/SymbolInterfaces.td"
include "mlir/Interfaces/CallInterfaces.td"
include "mlir/Interfaces/CastInterfaces.td"
include "mlir/Interfaces/ControlFlowInterfaces.td"
include "mlir/Interfaces/InferTypeOpInterface.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "mlir/IR/OpAsmInterface.td"

// Needed for `[NoMemoryEffect]`
// Note that Host.h also needs to include "mlir/Interfaces/SideEffectInterfaces.h" !
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/Host/IR/HostDialect.td"
include "Dialect/Host/IR/HostTypes.td"


class Host_Op<string mnemonic, list<Trait> traits = []> :
  Op<Host_Dialect, mnemonic, traits>;


def HostConstantOp : Host_Op<"constant"> {
    let arguments = (ins I32Attr:$value);

    let results = (outs I32);
}


#endif // HOST_OPS_TD