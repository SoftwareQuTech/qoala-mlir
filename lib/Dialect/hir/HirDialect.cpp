#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Types.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/InliningUtils.h"
#include "llvm/ADT/TypeSwitch.h"

#include "Dialect/hir/HirDialect.h"
#include "Dialect/hir/HirOps.h"
#include "Dialect/hir/HirTypes.h"

#include "Dialect/hir/HirOpsDialect.cpp.inc"

using namespace mlir;
using namespace mlir::hir;

void HirDialect::initialize()
{
    addOperations<
#define GET_OP_LIST
#include "Dialect/hir/HirOps.cpp.inc"
        >();
    addTypes<
#define GET_TYPEDEF_LIST
#include "Dialect/hir/HirOpsTypes.cpp.inc"
        >();
}

namespace
{

    static void print(QubitType qubitType, DialectAsmPrinter &os)
    {
        os << "qubit";
    }
    static Type parseQubit(DialectAsmParser &parser, MLIRContext *ctx)
    {
        if (failed(parser.parseLess()))
            return Type();

        return QubitType();
    }

} // namespace

#define GET_TYPEDEF_CLASSES
#include "Dialect/hir/HirOpsTypes.cpp.inc"

Type HirDialect::parseType(DialectAsmParser &parser) const
{
    StringRef mnemonic;
    if (failed(parser.parseKeyword(&mnemonic)))
        return Type();
    Type type;
    generatedTypeParser(parser, mnemonic, type);
    return type;
}

/// Print a type registered to this dialect.
void HirDialect::printType(Type type, DialectAsmPrinter &os) const
{
    (void)generatedTypePrinter(type, os);
}
