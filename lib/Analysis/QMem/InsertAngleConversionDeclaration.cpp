#include "mlir/IR/BuiltinOps.h"
#include "Dialect/Helpers/MIRToLIRHelperPasses.h"
#include "Dialect/QMem/QMem.h"

#include "Conversion/QoalaMIRToQoalaLIR/QoalaMIRToQoalaLIRPatterns.h"

#include "llvm/Support/Debug.h"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::helpers;
using namespace qoala::analysis;
using namespace qoala::conversion;

#define DEBUG_TYPE "f32-flattening"


namespace qoala::analysis {
#define GEN_PASS_DEF_ANGLECONVERSIONDECLARATION
#include "Dialect/Helpers/HelperPasses.h.inc"

    class InsertAngleConvertionDeclarationPass : public impl::AngleConversionDeclarationBase<InsertAngleConvertionDeclarationPass> {
    public:
        using AngleConversionDeclarationBase::AngleConversionDeclarationBase;
        void runOnOperation() override;
    };

    void InsertAngleConvertionDeclarationPass::runOnOperation() {
        ModuleOp module = this->getOperation();
        LLVM_DEBUG(llvm::dbgs() << "Inserting builtin angle conversion function declaration\n");
        angle::insertAngleConversionFunctionDeclaration(module);
    }
} /* namespace qoala::analysis */
