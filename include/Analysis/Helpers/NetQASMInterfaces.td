#ifndef QMEM_IFACE_TD
#define QMEM_IFACE_TD

include "mlir/IR/Interfaces.td"

def NetQASMRoutineInterface : OpInterface<"NetQASMRoutineInterface"> {
    let description = [{
        Interface for NetQASM operations that contain a region.
    }];
    let cppNamespace = "qoala::helpers";
    let methods = [
        InterfaceMethod<[{
            Returns the return operation of this routine
        }], "std::optional<mlir::Operation *>", "getReturnOperation"
        >,
        InterfaceMethod<[{
            Returns the an Array with the Types of the arguments of this routine
        }], "llvm::MutableArrayRef<mlir::BlockArgument>", "getArgsTypesList"
        >
    ];
}

def ExtraRegionVerifierInterface : OpInterface<"ExtraRegionVerifierInterface"> {
    let description = [{
        Interface for NetQASM operations that define an extra region verifier method.
    }];
    let cppNamespace = "qoala::helpers";
    let methods = [
        InterfaceMethod<[{
            Method used to perform extra verification of the region. Override this method to implement it.
        }], "mlir::LogicalResult", "validateNestedInstructions",
        (ins), /*methodBody=*/[{}],
        /*defaultImplementation=*/[{
            return mlir::LogicalResult::success();
        }]
        >
    ];
}

def MeasOp : OpInterface<"MeasOp"> {
    let description = [{
        Interface for NetQASM measure operations.
    }];
    let cppNamespace = "qoala::dialects::netqasm::ifaces";
}

def SingleQubitOp : OpInterface<"SingleQubitOp"> {
    let description = [{
        Interface for NetQASM operations acting on a single gate.
    }];
    let cppNamespace = "qoala::dialects::netqasm::ifaces";
}

def DualQubitOp : OpInterface<"DualQubitOp"> {
    let description = [{
        Interface for NetQASM operations acting on two gates.
    }];
    let cppNamespace = "qoala::dialects::netqasm::ifaces";
}

def EntangledQubitOp : OpInterface<"EntangledQubitOp"> {
    let description = [{
        Interface for NetQASM operations generating EPRs.
    }];
    let cppNamespace = "qoala::dialects::netqasm::ifaces";
}
#endif // QMEM_IFACE_TD