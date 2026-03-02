#include <set>

#include "Analysis/Helpers/Helpers.h"
#include "Analysis/QoalaHost/RemoteIDs.h"
#include "Dialect/Helpers/DialectHelpers.h"
#include "Dialect/QoalaHost/QoalaHost.h"

#include "Tools/QoalaOpt.h"
#include "llvm/ADT/StringSet.h"
#include "mlir/Interfaces/FunctionImplementation.h"

using namespace mlir;
using namespace qoala::dialects;
using namespace qoala::helpers;
using namespace qoala::analysis::reordering;

#define DEBUG_TYPE "qoala-host-helper"

#include "Dialect/NetQASM/NetQASM.h"
// include generated source code for operations
#define GET_OP_CLASSES
#include "Dialect/QoalaHost/QoalaHost.cpp.inc"

// include generated source code for types
#define GET_TYPEDEF_CLASSES
#include "Dialect/QoalaHost/QoalaHostTypes.cpp.inc"

/* Parse and print functions "ported" from func.func: parse and print */
ParseResult qoalahost::MainFuncOp::parse(OpAsmParser &parser, OperationState &result) {
    auto buildFuncType = [](Builder &builder, ArrayRef<Type> argTypes, ArrayRef<Type> results,
                            function_interface_impl::VariadicFlag,
                            std::string &) { return builder.getFunctionType(argTypes, results); };

    return function_interface_impl::parseFunctionOp(parser, result, /*allowVariadic=*/false,
                                                    getFunctionTypeAttrName(result.name), buildFuncType,
                                                    getArgAttrsAttrName(result.name), getResAttrsAttrName(result.name));
}

void qoalahost::MainFuncOp::build(OpBuilder &builder, OperationState &state, StringRef name, FunctionType type,
                                  ArrayRef<NamedAttribute> attrs, ArrayRef<DictionaryAttr> argAttrs) {
    state.addAttribute(SymbolTable::getSymbolAttrName(), builder.getStringAttr(name));
    state.addAttribute(getFunctionTypeAttrName(state.name), TypeAttr::get(type));
    state.attributes.append(attrs.begin(), attrs.end());
    state.addRegion();

    if (argAttrs.empty()) {
        return;
    }
    assert(type.getNumInputs() == argAttrs.size());
    function_interface_impl::addArgAndResultAttrs(builder, state, argAttrs, /*resultAttrs=*/std::nullopt,
                                                  getArgAttrsAttrName(state.name), getResAttrsAttrName(state.name));
}

void qoalahost::MainFuncOp::print(OpAsmPrinter &p) {
    function_interface_impl::printFunctionOp(p, *this, /*isVariadic=*/false, getFunctionTypeAttrName(),
                                             getArgAttrsAttrName(), getResAttrsAttrName());
}

static bool shouldCheckForRemoteRefsBlock(ModuleOp &module, qoalahost::MainFuncOp *mainFunc) {
    if (module.getOps<qremote::RemoteOp>().empty()) {
        return false;
    }

    return !mainFunc->getOps<qoalahost::ifaces::ClassicalCommInterface>().empty();
}

/* Region verifiers for MainFuncOp */
LogicalResult qoalahost::MainFuncOp::verifyRegions() {
    for (Operation &operation : this->getBody().getOps()) {
        auto name = operation.getName().getStringRef().str();
        if (QoalaHostDialect::opIsNotFromAllowedDialects(operation)) {
            return this->emitOpError() << "'" << getOperationName() << "' "
                                       << "op contains an operation that is not from  the allowed list of dialects: ["
                                       << QoalaHostDialect::getAllowedDialectNames() << "]. Operation: '" << operation
                                       << "'";
        }
    }

    // Verification of qoalahost.blk_meta sanity.
    // 1. There must exactly one qoalahost.blk_meta operation per block
    // 2. The qoalahost.blk_meta operation is always the first one of its block
    // 3. A block whose identifier is present in the blk_meta of another must be declared before
    std::set<std::string> blkIds;
    for (Block &block : getBody()) {
        auto blkMetas = block.getOps<BlkMeta>();

        auto it = blkMetas.begin();
        auto end = blkMetas.end();

        if (it == end) {
            return this->emitOpError()
                   << "each block must contain exactly one 'qoalahost.blk_meta' operation, but found none.";
        }

        if (std::next(it) != end) {
            return this->emitOpError()
                   << "each block must contain exactly one 'qoalahost.blk_meta' operation, but found multiple.";
        }

        BlkMeta op = *it;
        if (&block.front() != op.getOperation()) {
            return op.emitOpError() << "must be the first operation in each block.";
        }

        // We also ensure that the blocks are defined is a sane order. A block can be a precedence of another one
        // iif it is declared first.
        for (StringRef pred : op.getPredecessorsAttr().getAsValueRange<StringAttr>()) {
            if (blkIds.find(pred.str()) == blkIds.end()) {
                return op.emitOpError() << "contains a predecessor before its declaration: '" << pred << "'.";
            }
        }
        for (StringRef pred : op.getDependenciesAttr().getAsValueRange<StringAttr>()) {
            if (blkIds.find(pred.str()) == blkIds.end()) {
                return op.emitOpError() << "contains a dependency before its declaration: '" << pred << "'.";
            }
        }
        std::string prevComm = op.getPrevCommAttr().getValue().str();
        if (!prevComm.empty() && !blkIds.count(prevComm)) {
            return op.emitOpError() << "contains a previous comm precedence before its declaration: '" << prevComm
                                    << "'.";
        }
        std::string prevEnt = op.getPrevEntAttr().getValue().str();
        if (!prevEnt.empty() && !blkIds.count(prevEnt)) {
            return op.emitOpError() << "contains a previous ent precedence before its declaration: '" << prevEnt
                                    << "'.";
        }

        DictionaryAttr deadlinesAttr = op.getDeadlinesAttr();
        LLVM_DEBUG(llvm::dbgs() << "**** DeadAttr" << deadlinesAttr << "\n");
        for (NamedAttribute pair : deadlinesAttr.getValue()) {
            std::string key(pair.getName().strref());
            if (blkIds.find(key) == blkIds.end()) {
                return op.emitOpError() << "contains a block identifier in deadlines before its declaration: '" << key
                                        << "'.";
            }
        }

        blkIds.insert(op.getBlockId().str());
    }

    // Verification of the RemoteIDRefOp block. *If there is a remote declared and used*, then there
    // should be one block which:
    // * MUST contain 1 blk_meta at the start
    // * MUST contain 1 NopTOp at the end
    // * All other operations in between *must* be RemoteIDRefOp
    auto module = this->getOperation()->getParentOfType<ModuleOp>();
    if (shouldCheckForRemoteRefsBlock(module, this)) {
        // There is at least one remote declared and used -> apply the validation of the remote refs block
        std::optional<Block *> remoteRefsBlock = analysis::remoteids::getRemoteRefsBlock(module);
        if (!remoteRefsBlock) {
            this->emitOpError("Remote references block was not found and remote is declared asn used.");
            return failure();
        }
        auto &blockOperations = remoteRefsBlock.value()->getOperations();
        Operation &firstOp = blockOperations.front();
        Operation &lastOp = blockOperations.back();

        if (blockOperations.size() < 3) {
            return this->emitOpError() << "Remote references block does not have enough operations (3+) to be valid.";
        }

        if (!isa<BlkMeta>(firstOp)) {
            return firstOp.emitError()
                   << "Remote references block: First operation of the block is not a qoalahost.blk_meta.";
        }

        for (Operation &op : blockOperations) {
            if (&op == &firstOp || &op == &lastOp) {
                continue;
            }
            if (!isa<RemoteIDRefOp>(op)) {
                return op.emitError() << "Remote references block contains an operation not allowed in there.";
            }
        }

        if (!isa<NopTOp>(lastOp)) {
            return lastOp.emitError()
                   << "Remote references block: Last operation of the block it not a qoalahost.nop_term.";
        }
    }

    return success();
}

/* Call operation verifier */
LogicalResult qoalahost::CallOp::verifySymbolUses(SymbolTableCollection &symbolTable) {
    // Search the symbol in the parent SymbolTables
    // The declared symbol MUST come from a netqasm.local_routine OR netqasm.request_routine operation
    if (symbolTable.lookupNearestSymbolFrom<netqasm::LocalRoutineOp>(this->getOperation(), this->getCalleeAttr())) {
        return success();
    }
    if (symbolTable.lookupNearestSymbolFrom<netqasm::RequestRoutineOp>(this->getOperation(), this->getCalleeAttr())) {
        return success();
    }
    return this->emitOpError() << "'" << this->getCalleeAttr().getAttr().str() << "' "
                               << "does not reference a valid defined by either netqasm.local_routine or "
                               << "netqasm.request_routine.";
}

/* Additional verifier for the Call operation to check number of arguments used. */
LogicalResult qoalahost::CallOp::verify() {
    auto module = this->getOperation()->getParentOfType<ModuleOp>();
    // Even if we write MLIR without declaring a module, MLIR will insert a top-level module operation, so
    // it is safe to assert the existence of the module.
    assert(module && "Unexpected - Operation not inside an MLIR module operation");
    if (const Operation *callee = helpers::getRoutineWithName(&module, this->getCallee()); !callee) {
        this->emitOpError() << "Called function '" << this->getCallee() << "' was not found in the module.";
        return failure();
    } else {
        auto netQASMRoutine = dyn_cast<NetQASMRoutineInterface>(callee);
        const MutableArrayRef<BlockArgument> routineArgs = netQASMRoutine.getArgsTypesList();
        if (this->getArgOperands().size() != routineArgs.size()) {
            this->emitError() << "Call operation does not match the number of arguments of the callee.";
            return failure();
        }
        for (uint32_t i = 0; i < this->getNumOperands(); i++) {
            const Type callOperandType = this->getOperand(i).getType();
            if (callOperandType != routineArgs[i].getType()) {
                this->emitOpError() << "Operand #" << i << " does not match the type of the callee.";
                return failure();
            }
        }
    }
    return success();
}

uint64_t qoalahost::RecvIntOp::getDuration() { return options::qoalaOptLatency + options::qoalaOptHostPeerLatency; }

uint64_t qoalahost::RecvFloatOp::getDuration() { return options::qoalaOptLatency + options::qoalaOptHostPeerLatency; }

BlockType qoalahost::SendIntOp::getBlockType(const llvm::StringMap<Operation *> &routineMap) { return BlockType::CC; }

BlockType qoalahost::RecvIntOp::getBlockType(const llvm::StringMap<Operation *> &routineMap) { return BlockType::CC; }

BlockType qoalahost::SendFloatOp::getBlockType(const llvm::StringMap<Operation *> &routineMap) { return BlockType::CC; }

BlockType qoalahost::RecvFloatOp::getBlockType(const llvm::StringMap<Operation *> &routineMap) { return BlockType::CC; }

uint64_t qoalahost::RecvIntsOp::getDuration() {
    // TODO - This need to be improved because this is just a rough estimation about how long does a recv op takes
    const uint32_t numberOfValues = static_cast<uint32_t>(this->getLengthAttr().getInt());
    return (options::qoalaOptLatency + options::qoalaOptHostPeerLatency) * numberOfValues;
}

uint64_t qoalahost::RecvFloatsOp::getDuration() {
    // TODO - This need to be improved because this is just a rough estimation about how long does a recv op takes
    const uint32_t numberOfValues = static_cast<uint32_t>(this->getLengthAttr().getInt());
    return (options::qoalaOptLatency + options::qoalaOptHostPeerLatency) * numberOfValues;
}

BlockType qoalahost::SendIntsOp::getBlockType(const llvm::StringMap<Operation *> &routineMap) { return BlockType::CC; }

BlockType qoalahost::RecvIntsOp::getBlockType(const llvm::StringMap<Operation *> &routineMap) { return BlockType::CC; }

BlockType qoalahost::SendFloatsOp::getBlockType(const llvm::StringMap<Operation *> &routineMap) {
    return BlockType::CC;
}

BlockType qoalahost::RecvFloatsOp::getBlockType(const llvm::StringMap<Operation *> &routineMap) {
    return BlockType::CC;
}

BlockType qoalahost::CallOp::getBlockType(const llvm::StringMap<Operation *> &routineMap) {
    const StringRef symName = this->getCallee();
    Operation *callee = routineMap.contains(symName) ? routineMap.at(symName) : nullptr;
    if (!callee) {
        return BlockType::CL;
    }

    if (isa<netqasm::RequestRoutineOp>(callee)) {
        return BlockType::QC;
    }
    if (isa<netqasm::LocalRoutineOp>(callee)) {
        return BlockType::QL;
    }
    return BlockType::CL;
}

Operation *qoalahost::CallOp::getCalleeOperation() {
    const auto calleeName = this->getCalleeAttr().getAttr();
    return SymbolTable::lookupNearestSymbolFrom(this->getOperation(), calleeName);
}

/* Helper functions from the QoalaHostDialect class */
bool qoalahost::QoalaHostDialect::opIsNotFromAllowedDialects(Operation &operation) {
    return !belongsToDialect<
#define GET_ALLOWED_DIALECTS
#include "Dialect/QoalaHost/QoalaHost.h"
            >(operation);
}

std::string qoalahost::QoalaHostDialect::getAllowedDialectNames() {
    return getDialectNamesList<
#define GET_ALLOWED_DIALECTS
#include "Dialect/QoalaHost/QoalaHost.h"
            >();
}
