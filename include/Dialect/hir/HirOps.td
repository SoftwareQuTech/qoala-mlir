#ifndef HIR_OPS_TD
#define HIR_OPS_TD

include "mlir/IR/OpBase.td"

// Needed for FunctionOpInterface
// Note that Hir.h also needs to include "mlir/Interfaces/FunctionInterfaces.h" !
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
// Note that Hir.h also needs to include "mlir/Interfaces/SideEffectInterfaces.h" !
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/hir/HirDialect.td"
include "Dialect/hir/HirTypes.td"


class Hir_Op<string mnemonic, list<Trait> traits = []> :
  Op<Hir_Dialect, mnemonic, traits>;


#endif // HIR_OPS_TD