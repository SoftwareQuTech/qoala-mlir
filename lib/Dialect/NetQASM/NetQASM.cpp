#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/IRMapping.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Support/LLVM.h"
#include "llvm/ADT/ArrayRef.h"
// #include "mlir/IR/FunctionImplementation.h"

#include "mlir/IR/DialectImplementation.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/NetQASM/NetQASM.h"
#include "Dialect/NetQASM/NetQASMDialect.h"

using namespace mlir;
using namespace qoala::dialects::netqasm;

// include generated source code for operations
#define GET_OP_CLASSES
#include "Dialect/NetQASM/NetQASM.cpp.inc"

LogicalResult EprsOp::verifySymbolUses(SymbolTableCollection &symbolTable) {
    // Check that the callee attribute was specified.
    auto fnAttr = (*this)->getAttrOfType<FlatSymbolRefAttr>("remote");
    if (!fnAttr)
        return emitOpError("requires a 'remote' symbol reference attribute");
    RemoteOp fn = symbolTable.lookupNearestSymbolFrom<RemoteOp>(*this, fnAttr);
    if (!fn)
        return emitOpError() << "'" << fnAttr.getValue()
                             << "' does not reference a valid remote node";

    return success();
}

LogicalResult
EprsMeasureOp::verifySymbolUses(SymbolTableCollection &symbolTable) {
    // Check that the callee attribute was specified.
    auto fnAttr = (*this)->getAttrOfType<FlatSymbolRefAttr>("remote");
    if (!fnAttr)
        return emitOpError("requires a 'remote' symbol reference attribute");
    RemoteOp fn = symbolTable.lookupNearestSymbolFrom<RemoteOp>(*this, fnAttr);
    if (!fn)
        return emitOpError() << "'" << fnAttr.getValue()
                             << "' does not reference a valid remote node";

    return success();
}