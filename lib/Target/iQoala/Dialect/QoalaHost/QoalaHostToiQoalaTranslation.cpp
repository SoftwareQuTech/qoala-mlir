#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"
#include "mlir/IR/Operation.h"
#include "Target/iQoala/QoalaTranslationInterface.h"
#include "Target/iQoala/ModuleTranslation.h"
#include "Target/iQoala/Dialect/QoalaHost/QoalaHostToiQoalaTranslation.h"

#include "Dialect/QoalaHost/QoalaHost.h"

#define DEBUG_TYPE "qoalahost-translation"

using namespace mlir;
using namespace qoala::dialects::qoalahost;
using namespace qoala::iqoala;

static LogicalResult translateBlock(mlir::Block &block, qoala::translate::ModuleTranslation &moduleTranslation) {
    for (Operation &op : block.getOperations()) {
        if (failed(moduleTranslation.convertOperation(op))) {
            return op.emitOpError("cannot covert operation '") << op << "'\n";
        }
    }
    return success();
}

static LogicalResult translateMainFunction(MainFuncOp &mainFuncOP, qoala::translate::ModuleTranslation &moduleTranslation) {
    moduleTranslation.setModuleName(mainFuncOP.getName());
    for (mlir::Block &block: mainFuncOP.getBlocks()) {
        if (failed(translateBlock(block, moduleTranslation))) {
            return mainFuncOP->emitOpError("cannot convert a block inside function '")
                    << mainFuncOP.getSymName() << "'\n";
        }
    }
    return success();
}

static LogicalResult translateQoalaHostOperation(Operation *operation, qoala::translate::ModuleTranslation &moduleTranslation) {
    // TODO - Implement this dispatcher
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
            .Case([&](MainFuncOp op) -> LogicalResult {
                return translateMainFunction(op, moduleTranslation);
            })
            .Case([](CallOp op) -> LogicalResult {
                return success();
            })
            .Case([](ReturnOp op) -> LogicalResult {
                return success();
            })
            .Case([](SendIntsOp op) -> LogicalResult {
                return success();
            })
            .Case([](SendFloatsOp op) -> LogicalResult {
                return success();
            })
            .Case([](RecvIntsOp op) -> LogicalResult {
                return success();
            })
            .Case([](RecvFloatsOp op) -> LogicalResult {
                return success();
            })
            .Default([](Operation *op) -> LogicalResult {
                return op->emitOpError("Unknown way to translate a QoalaHost operation to iQoala: '") << *op << "'\n";
            });
}

namespace qoala::translate {
    class QoalaHostToiQoalaTranslationInterface : public QoalaTranslationDialectInterface {
    public:
        using QoalaTranslationDialectInterface::QoalaTranslationDialectInterface;

        LogicalResult convertOperation(Operation *op, ModuleTranslation &moduleTranslation) const final {
            return translateQoalaHostOperation(op, moduleTranslation);
        }

    };

    void registerQoalaHostToiQoalaTranslations(DialectRegistry &registry) {
        registry.insert<QoalaHostDialect>();
        registry.addExtension(+[](MLIRContext *ctx, QoalaHostDialect *dialect) {
            dialect->addInterfaces<QoalaHostToiQoalaTranslationInterface>();
        });
    }
}