#include "llvm/ADT/TypeSwitch.h"
#include "mlir/IR/Operation.h"
#include "Target/iQoala/ModuleTranslation.h"
#include "Target/iQoala/Dialect/QoalaHost/QoalaHostToiQoalaTranslation.h"
#include "Dialect/QoalaHost/QoalaHost.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "qoalahost-translation"

using namespace mlir;
using namespace qoala::translate;
using namespace qoala::dialects::qoalahost;

static LogicalResult translateBlock(Block &block, ModuleTranslation *moduleTranslation) {
    (void) moduleTranslation->emplaceNewBlockInHostSection(&block);
    for (Operation &op : block.getOperations()) {
        if (failed(moduleTranslation->convertOperation(op))) {
            return op.emitOpError("cannot covert operation '") << op << "'\n";
        }
    }
    return success();
}

static LogicalResult translateMainFunction(MainFuncOp &mainFuncOP, ModuleTranslation *moduleTranslation) {
    moduleTranslation->setModuleName(mainFuncOP.getName());
    for (Block &block : mainFuncOP.getBlocks()) {
        if (failed(translateBlock(block, moduleTranslation))) {
            return mainFuncOP->emitOpError("cannot convert a block inside function '")
                    << mainFuncOP.getSymName() << "'\n";
        }
    }
    return success();
}

static LogicalResult translateQoalaHostOperation(Operation *operation, ModuleTranslation *moduleTranslation) {
    // TODO - Implement this dispatcher
    LLVM_DEBUG(llvm::dbgs() << "******** Translating op '" << operation->getName() << "' *********\n");
    return llvm::TypeSwitch<Operation *, LogicalResult>(operation)
            .Case([&](MainFuncOp op) -> LogicalResult {
                return translateMainFunction(op, moduleTranslation);
            })
            .Case([&](CallOp op) -> LogicalResult {
                // std::string callee = op.getCallee().str();
                // for (auto kk : op.getArgOperands()) {
                //     // TODO - Create an expression for the arguments
                // }
                // qoala::iqoala::Block *iQoalaBlock = moduleTranslation.getMappediQoalaBlock(op->getBlock());
                return success();
            })
            .Case([](ReturnOp op) -> LogicalResult {
                return success();
            })
            .Case([](SendIntsOp op) -> LogicalResult {
                return success();
            })
            .Case([](SendFloatsOp op) -> LogicalResult {
                return op->emitOpError("Sending floats is not supported yet: '") << *op << "'\n";
            })
            .Case([](RecvIntsOp op) -> LogicalResult {
                return success();
            })
            .Case([](RecvFloatsOp op) -> LogicalResult {
                return op->emitOpError("Receiving floats is not supported yet: '") << *op << "'\n";
            })
            .Default([](Operation *op) -> LogicalResult {
                return op->emitOpError("Unknown way to translate a QoalaHost operation to iQoala: '") << *op << "'\n";
            });
}

namespace qoala::translate {
    LogicalResult QoalaHostToiQoalaTranslation::convertOperation(Operation *op, ModuleTranslation *moduleTranslation) const {
        return translateQoalaHostOperation(op, moduleTranslation);
    }
}