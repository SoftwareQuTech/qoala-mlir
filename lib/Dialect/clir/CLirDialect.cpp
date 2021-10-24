#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/clir/CLirDialect.h"
#include "Dialect/clir/CLirOps.h"
#include "Dialect/clir/CLirTypes.h"

#include "Dialect/clir/CLirOpsDialect.cpp.inc"

using namespace mlir;
using namespace mlir::clir;

void CLirDialect::initialize()
{
    addOperations<
#define GET_OP_LIST
#include "Dialect/clir/CLirOps.cpp.inc"
        >();
    addTypes<
#define GET_TYPEDEF_LIST
#include "Dialect/clir/CLirOpsTypes.cpp.inc"
        >();
}

#define GET_TYPEDEF_CLASSES
#include "Dialect/clir/CLirOpsTypes.cpp.inc"

Type CLirDialect::parseType(DialectAsmParser &parser) const
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
void CLirDialect::printType(Type type, DialectAsmPrinter &os) const
{
    (void)generatedTypePrinter(type, os);
}
