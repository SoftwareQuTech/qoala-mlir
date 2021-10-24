#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/qlir/QLirDialect.h"
#include "Dialect/qlir/QLirOps.h"
#include "Dialect/qlir/QLirTypes.h"

#include "Dialect/lircommon/LirCommonTypes.h"

#include "Dialect/qlir/QLirOpsDialect.cpp.inc"

using namespace mlir;
using namespace mlir::qlir;

void QLirDialect::initialize()
{
    addOperations<
#define GET_OP_LIST
#include "Dialect/qlir/QLirOps.cpp.inc"
        >();
    addTypes<
#define GET_TYPEDEF_LIST
#include "Dialect/qlir/QLirOpsTypes.cpp.inc"
        >();
}

#define GET_TYPEDEF_CLASSES
#include "Dialect/qlir/QLirOpsTypes.cpp.inc"

Type QLirDialect::parseType(DialectAsmParser &parser) const
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
void QLirDialect::printType(Type type, DialectAsmPrinter &os) const
{
    (void)generatedTypePrinter(type, os);
}
