#include "llvm/Support/raw_ostream.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/IR/BuiltinOps.h"

#include "Dialect/hir/Hir.h"
#include "Dialect/hir/Passes.h"

namespace mlir
{
#define GEN_PASS_DEF_HIRCHECKLINEAR
#include "Dialect/hir/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace mlir::hir;

namespace
{

  struct HirCheckLinearPass
      : public impl::HirCheckLinearBase<HirCheckLinearPass>
  {
    void runOnOperation() override;
  };

} // namespace

/*
 * Helper function to quickly check if a mlir::Operation* can be
 * casted into one of the Op classes provided as a list.
 */
template <typename T, typename... Rest>
bool tryCast(mlir::Operation *op)
{
  if (auto castedOp = llvm::dyn_cast<T>(op))
  {
    return true;
  }
  else if constexpr (sizeof...(Rest) > 0)
  {
    return tryCast<Rest...>(op);
  }
  return false;
}

bool isQuantumOp(mlir::Operation *op)
{
  return tryCast<NewQubitOp, RotXOp, RotYOp, RotZOp, HadamardOp, CnotOp, CzOp,
                 CrotXOp, MeasureOp, EprsOp, EprsMeasureOp>(op);
}

void HirCheckLinearPass::runOnOperation()
{
  // llvm::outs() << "\n=== Start of CheckLinear pass ===\n\n";

  Operation *operation = getOperation();

  if (ModuleOp module = llvm::dyn_cast<ModuleOp>(operation))
  {
    // Walk over all ops in the module.
    module.walk([](Operation *op)
                {
      if (isQuantumOp(op))
      {
        // For all ops that produce quantum values, check the results.
        for (auto result : op->getResults())
        {
          // Check if the result value has zero or one use.
          if (!result.hasOneUse() && !result.use_empty())
          {
            // More than 1 use.
            op->emitError("Result of operation is used more than once.\n");
            for (auto &use : result.getUses())
            {
              auto usingOp = use.getOwner();
              emitError(usingOp->getLoc(), "Used here\n");
            }
          }
        }
      } });
  }
  else
  {
    llvm::errs() << "Not a ModuleOp: something went wrong.\n";
  }

  // llvm::outs() << "\n=== End of CheckLinear pass ===\n\n";
}

std::unique_ptr<Pass> mlir::createHirCheckLinearPass()
{
  return std::make_unique<HirCheckLinearPass>();
}
