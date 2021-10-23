#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/StandardOps/IR/Ops.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Translation.h"

#include "llvm/Support/ToolOutputFile.h"
#include <string>
#include <vector>

#include "Dialect/lir/LirDialect.h"
#include "Dialect/lir/LirOps.h"
#include "Target/lir/ConvertToLir.h"
#include "Dialect/hir/HirDialect.h"

using namespace mlir;
using namespace lir;

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
                registry.insert<lir::LirDialect, hir::HirDialect, StandardOpsDialect>();
            });
    }
} // namespace mlir
