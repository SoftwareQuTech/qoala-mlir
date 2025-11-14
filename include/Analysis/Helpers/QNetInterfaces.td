#ifndef QNET_IFACE_TD
#define QNET_IFACE_TD

include "mlir/IR/Interfaces.td"

def QubitOpIface : OpInterface<"QubitOpIface"> {
    let description = [{
        Interface for operations with qubit operands and qubit results.
    }];
    let cppNamespace = "qoala::helpers";
    let methods = [
        InterfaceMethod<[{
        Return the operation name.
        }], "mlir::OperationName", "getName", (ins),
            /*methodBody=*/[{}],
            /*defaultImplementation=*/[{
            auto *op = $_op.getOperation();
            return op->getName();
            }] >,
        
        InterfaceMethod<[{
        Return whether this is a two-qubit operation.
        }], "bool", "isTwoQubitOp", (ins),
            /*methodBody=*/[{}],
            /*defaultImplementation=*/[{
            // Default: most operations are single-qubit, return false
            return false;
            }] >,
        
        InterfaceMethod<[{
        Return all of the qubit operands.
        // Assume that number of qubit operands equals number of results.
        // Override for NewQubitOp and EprsOp.
        }], "mlir::OperandRange", "getQubitOperands", (ins),
            /*methodBody=*/[{}],
            /*defaultImplementation=*/[{
            auto *op = $_op.getOperation();
            uint32_t numQubitsRes = op->getNumResults();
            return op->getOperands().take_front(numQubitsRes);
            }]>,
        
        InterfaceMethod<[{
        Return all of the qubit results.
        // Assume that QubitOpIface returns only qubit results.
        }], "mlir::ResultRange", "getQubitResults", (ins),
            /*methodBody=*/[{}],
            /*defaultImplementation=*/[{
            auto *op = $_op.getOperation();
            uint32_t numQubitsRes = op->getNumResults();
            return op->getResults().take_front(numQubitsRes);
            }]>
    ];
}

def HermitianOpIface : OpInterface<"HermitianOpIface"> {
    let description = [{
        Interface for hermitian operations.
    }];
    let cppNamespace = "qoala::helpers";
    let methods = [];
}

def RotationOpIface : OpInterface<"RotationOpIface"> {
    let description = [{
        Interface for rotation operations (single-qubit Rx/Ry/Rz and controlled
        rotations). Provides accessors for axis, target, (optional) controls,
        and the rotation angle attribute.
    }];
    let cppNamespace = "qoala::helpers";

    let methods = [
        InterfaceMethod<[{
        Return the rotation axis (X/Y/Z).
        }], "qoala::helpers::RotationAxis", "getAxis", (ins),
            /*methodBody=*/[{}] >,

        InterfaceMethod<[{
        Return the target qubit value.
        }], "mlir::Value", "getTarget", (ins),
            /*methodBody=*/[{}],
            /*defaultImplementation=*/[{
            auto *op = $_op.getOperation();
            uint32_t n = op->getNumOperands();
            assert(n >= 2 && "RotationOpIface: need at least [target, angle]");
            return op->getOperand(n - 2);
            }] >,

        InterfaceMethod<[{
        Return the control qubits (empty if none).
        }], "mlir::ValueRange", "getControls", (ins),
            /*methodBody=*/[{}],
            /*defaultImplementation=*/[{
            auto *op = $_op.getOperation();
            uint32_t n = op->getNumOperands();
            assert(n >= 2 && "RotationOpIface: need at least [target, angle]");
            uint32_t tgtIdx = n - 2;
            return op->getOperands().take_front(tgtIdx); // OK
            }] >,

        InterfaceMethod<[{
        Get the rotation angle as an attribute if constant; otherwise null.
        }], "mlir::Attribute", "getAngleAttr", (ins),
            /*methodBody=*/[{}],
            /*defaultImplementation=*/[{
            auto *op = $_op.getOperation();
            uint32_t n = op->getNumOperands();
            assert(n >= 1 && "RotationOpIface: no operands");
            mlir::Value angleVal = op->getOperand(n - 1);
            if (auto cst = angleVal.getDefiningOp<mlir::arith::ConstantOp>()) {
                if (auto fa = llvm::dyn_cast<mlir::FloatAttr>(cst.getValue()))
                return fa;
                return cst.getValue();
            }
            return mlir::Attribute();
            }] >,

        // ---- NO DEFAULT HERE (avoid $angle substitution) ----
        InterfaceMethod<[{
        Set the rotation angle operand from an attribute (expects FloatAttr).
        }], "void", "setAngleAttr", (ins "mlir::Attribute":$angle),
            /*methodBody=*/[{}] >,

        InterfaceMethod<[{
        Return the operand index of the target qubit.
        }], "uint32_t", "getTargetOperandIndex", (ins),
            /*methodBody=*/[{}],
            /*defaultImplementation=*/[{
            auto *op = $_op.getOperation();
            uint32_t n = op->getNumOperands();
            assert(n >= 2 && "RotationOpIface: need at least [target, angle]");
            return n - 2u;
            }] >
    ];
}


#endif // QNET_IFACE_TD