#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/StandardOps/IR/Ops.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Translation.h"

#include "llvm/Support/ToolOutputFile.h"
#include <string>
#include <vector>

#include "Dialect/clir/CLirDialect.h"
#include "Dialect/clir/CLirOps.h"
#include "Dialect/qlir/QLirDialect.h"
#include "Dialect/qlir/QLirOps.h"
#include "Dialect/lircommon/LirCommonDialect.h"
#include "Dialect/lircommon/LirCommonOps.h"
#include "Target/lir/ConvertToText.h"
#include "Dialect/hir/HirDialect.h"

using namespace mlir;
using namespace clir;
using namespace qlir;
using namespace lircommon;

namespace mlir
{
    void registerToLirTranslation()
    {
        [[maybe_unused]] TranslateFromMLIRRegistration registration(
            "hir-to-lir",
            [](ModuleOp module, raw_ostream &output)
            {
                // return QASMTranslation(module, output).translate();
                module.print(output);
                return success();
            },
            [](DialectRegistry &registry)
            {
                registry.insert<lircommon::LirCommonDialect, qlir::QLirDialect, clir::CLirDialect, hir::HirDialect, StandardOpsDialect>();
            });
    }
} // namespace mlir
