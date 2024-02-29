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

void checkSingleUse(Operation *op)
{
  // Check if the result value has zero or one use.
  auto result = op->getResult(0);
}

void HirCheckLinearPass::runOnOperation()
{
  llvm::outs() << "\n=== Start of CheckLinear pass ===\n\n";

  Operation *operation = getOperation();

  if (ModuleOp module = llvm::dyn_cast<ModuleOp>(operation))
  {
    // Walk over all ops in the module.
    module.walk([](Operation *op)
                {
                  std::string msgBuffer = "";
                  llvm::raw_string_ostream msgStream(msgBuffer); 

                  msgStream << "printing results for: ";
                  op->print(msgStream);
                  msgStream << "\n";
                  msgStream.flush();
                  llvm::outs() << msgBuffer;
                  for (auto result : op->getResults())
                  {
                    result.dump();

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

                  // if (auto opp = llvm::dyn_cast<NewQubitOp>(op))
                  // {
                  //   checkSingleUse(opp);
                  // }
                  // else if (auto opp = llvm::dyn_cast<RotXOp>(op))
                  // {
                  //   checkSingleUse(opp);
                  // }
                });
  }
  else
  {
    llvm::errs() << "Not a ModuleOp: something went wrong.\n";
  }

  llvm::outs() << "\n=== End of CheckLinear pass ===\n\n";
}

std::unique_ptr<Pass> mlir::createHirCheckLinearPass()
{
  return std::make_unique<HirCheckLinearPass>();
}
