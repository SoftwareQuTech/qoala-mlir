#include "mlir/IR/AsmState.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Pass/PassRegistry.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "Dialect/hir/Hir.h"
#include "Dialect/hir/HirDialect.h"

#include "Dialect/hir/Passes.h"

#include "Dialect/QMem/QMem.h"
#include "Dialect/QMem/QMemDialect.h"

// Since the lowering pass is part of this opt tool,
// we need to also link the libraries of the LIR dialect
#include "Dialect/lir/Lir.h"
#include "Dialect/lir/LirDialect.h"

// And, of course, we also need the libraries of the lowering pass itself
#include "lowering/HIRToLIR.h"

int main(int argc, char **argv) {
  mlir::DialectRegistry registry;
  // We register all the default dialects from MLIR
  registerAllDialects(registry);
  // We register all the dialects we are implementing
  registry.insert<mlir::hir::HirDialect>();
  registry.insert<mlir::lir::LirDialect>();
  registry.insert<mlir::qmem::QMemDialect>();

  // We also register all the passes from MLIR
  mlir::registerAllPasses();
  // And also the passes from HIR
  mlir::registerHirPasses();
  // And the pass that lowers HIR to LIR
  mlir::registerHirToLirPasses();

  mlir::registerViewOpGraphPass();

  mlir::MLIRContext context;

  mlir::PassManager pm(&context);

  return failed(mlir::MlirOptMain(argc, argv, "Qoala optimizer\n", registry));
}
