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

namespace
{

    class NetQASMTranslation
    {
        ModuleOp module;
        raw_ostream &output;

        DenseMap<Value, std::string> scope;
        DenseMap<uint8_t, uint8_t> reg_vals;
        DenseMap<Value, uint8_t> phys_qubits;
        uint8_t phys_qubits_in_use = 0;

        // Helpers
        void addQubit(Value qubit);

        uint8_t newQubit(Value qubit);
        uint8_t mapQubit(Value qubit, Value phys_id);
        uint8_t getPhysId(Value qubit);

        std::string getRegForValue(uint8_t value);
        bool hasRegForValue(uint8_t value);

        void addCreg(Value qubit);
        StringRef lookupScope(Value v) { return scope[v]; }
        void splitArguments(ArrayRef<Value> args, SmallVector<Value> &params,
                            SmallVector<Value> &qubits);

        std::string flattenExpr(Value val);
        std::string flattenExprBraced(Value val);

        template <typename T>
        WalkResult runPrinters(Operation *op, StringRef prefix = "")
        {
            if (auto opp = dyn_cast<T>(op))
            {
                output << prefix;
                if (failed(print(opp)))
                    return WalkResult::interrupt();
            }
            return WalkResult::advance();
        }
        template <typename T, typename V, typename... Ts>
        WalkResult runPrinters(Operation *op, StringRef prefix = "")
        {
            if (auto opp = dyn_cast<T>(op))
            {
                output << prefix;
                if (failed(print(opp)))
                    return WalkResult::interrupt();
                return WalkResult::advance();
            }
            return runPrinters<V, Ts...>(op, prefix);
        }

        // Op printers
        LogicalResult print(lircommon::AllocateOp op);
        LogicalResult print(lircommon::EntangleOp op);
        LogicalResult print(lircommon::GateXOp op);
        LogicalResult print(lircommon::GateYOp op);
        LogicalResult print(lircommon::GateZOp op);
        LogicalResult print(lircommon::MeasOp op);

    public:
        NetQASMTranslation(ModuleOp op, raw_ostream &output)
            : module(op), output(output) {}
        LogicalResult translate();
    };

    void NetQASMTranslation::addQubit(Value qubit)
    {
        int idx = scope.size();
        scope[qubit] = std::to_string(idx);
    }

    uint8_t NetQASMTranslation::newQubit(Value qubit)
    {
        phys_qubits[qubit] = phys_qubits_in_use++;
        return phys_qubits_in_use - 1;
    }

    uint8_t NetQASMTranslation::mapQubit(Value qubit, Value new_qubit)
    {
        auto phys_id = phys_qubits[qubit];
        phys_qubits[new_qubit] = phys_id;
        return phys_id;
    }

    uint8_t NetQASMTranslation::getPhysId(Value qubit)
    {
        return phys_qubits[qubit];
    }

    LogicalResult NetQASMTranslation::print(lircommon::AllocateOp op)
    {
        auto phys_id = newQubit(op.qout());
        output << "set Q" << std::to_string(phys_id) << " " << phys_id << "\n";
        output << "qalloc Q" << std::to_string(phys_id) << "\n";
        return success();
    }

    LogicalResult NetQASMTranslation::print(lircommon::GateXOp op)
    {
        auto phys_id = mapQubit(op.qin(), op.qout());
        output << "rot_x Q" << std::to_string(phys_id) << " <angle>"
               << "\n";
        return success();
    }

    LogicalResult NetQASMTranslation::print(lircommon::GateYOp op)
    {
        auto phys_id = mapQubit(op.qin(), op.qout());
        output << "rot_y Q" << std::to_string(phys_id) << " <angle>"
               << "\n";
        return success();
    }

    LogicalResult NetQASMTranslation::print(lircommon::GateZOp op)
    {
        auto phys_id = mapQubit(op.qin(), op.qout());
        output << "rot_z Q" << std::to_string(phys_id) << " <angle>"
               << "\n";
        return success();
    }

    LogicalResult NetQASMTranslation::print(lircommon::MeasOp op)
    {
        auto phys_id = getPhysId(op.qin());
        output << "meas Q" << std::to_string(phys_id) << " M0\n";
        return success();
    }

    LogicalResult NetQASMTranslation::print(lircommon::EntangleOp op)
    {
        addQubit(op.qout());
        output << "array @0 10\n";
        output << "array @1 10\n";
        output << "array @2 10\n";
        output << "create_epr(0, 1) 0 1 2\n";
        output << "wait_all @[0:10]\n";
        return success();
    }

    LogicalResult NetQASMTranslation::translate()
    {
        output << "# NetQASM 1.0\n";
        output << "# APPID 0\n\n";

        WalkResult result = module.walk<WalkOrder::PreOrder>(
            [&](Operation *op)
            {
                if (op->getName().getStringRef() == "lircommon.cval_c_to_q" ||
                    op->getName().getStringRef() == "lircommon.cval_q_to_c")
                {
                    // new subroutine
                    output << "-----------------------\n\n";
                    output << "# NetQASM 1.0\n";
                    output << "# APPID 0\n\n";
                    return WalkResult::advance();
                }
                else
                {
                    return runPrinters<
                        lircommon::AllocateOp,
                        lircommon::EntangleOp,
                        lircommon::GateXOp,
                        lircommon::GateYOp,
                        lircommon::GateZOp,
                        lircommon::MeasOp>(op);
                }
            });

        if (result.wasInterrupted())
            return failure();
        return success();
    }

} // namespace

namespace mlir
{
    void registerToLirTranslation()
    {
        [[maybe_unused]] TranslateFromMLIRRegistration registration(
            "lir-to-netqasm",
            [](ModuleOp module, raw_ostream &output)
            {
                return NetQASMTranslation(module, output).translate();
                // module.print(output);
                return success();
            },
            [](DialectRegistry &registry)
            {
                registry.insert<lircommon::LirCommonDialect, qlir::QLirDialect, clir::CLirDialect, hir::HirDialect, StandardOpsDialect>();
            });
    }
} // namespace mlir
