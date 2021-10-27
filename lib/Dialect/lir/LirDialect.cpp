#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/lir/LirDialect.h"
#include "Dialect/lir/LirOps.h"
#include "Dialect/lir/LirTypes.h"

#include "Dialect/lir/LirOpsDialect.cpp.inc"

using namespace mlir;
using namespace mlir::lir;

void LirDialect::initialize()
{
    addOperations<
#define GET_OP_LIST
#include "Dialect/lir/LirOps.cpp.inc"
        >();
    addTypes<
#define GET_TYPEDEF_LIST
#include "Dialect/lir/LirOpsTypes.cpp.inc"
        >();
}

#define GET_TYPEDEF_CLASSES
#include "Dialect/lir/LirOpsTypes.cpp.inc"

Type LirDialect::parseType(DialectAsmParser &parser) const
{
    StringRef mnemonic;
    if (failed(parser.parseKeyword(&mnemonic)))
    {
        parser.emitError(parser.getNameLoc(), "unknown type ?!!");
        return Type();
    }
    Type type;
    generatedTypeParser(parser, mnemonic, type);
    return type;
}

/// Print a type registered to this dialect.
void LirDialect::printType(Type type, DialectAsmPrinter &os) const
{
    (void)generatedTypePrinter(type, os);
}
