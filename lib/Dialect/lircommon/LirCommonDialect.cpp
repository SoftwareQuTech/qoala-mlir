#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/lircommon/LirCommonDialect.h"
#include "Dialect/lircommon/LirCommonOps.h"
#include "Dialect/lircommon/LirCommonTypes.h"

#include "Dialect/lircommon/LirCommonOpsDialect.cpp.inc"

using namespace mlir;
using namespace mlir::lircommon;

void LirCommonDialect::initialize()
{
    addOperations<
#define GET_OP_LIST
#include "Dialect/lircommon/LirCommonOps.cpp.inc"
        >();
    addTypes<
#define GET_TYPEDEF_LIST
#include "Dialect/lircommon/LirCommonOpsTypes.cpp.inc"
        >();
}

#define GET_TYPEDEF_CLASSES
#include "Dialect/lircommon/LirCommonOpsTypes.cpp.inc"

Type LirCommonDialect::parseType(DialectAsmParser &parser) const
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
void LirCommonDialect::printType(Type type, DialectAsmPrinter &os) const
{
    (void)generatedTypePrinter(type, os);
}
