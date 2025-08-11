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
        }], "mlir::Operation *", "getReturnOperation"
        >,
        InterfaceMethod<[{
            Returns the an Array with the Types of the arguments of this routine
        }], "llvm::MutableArrayRef<mlir::BlockArgument>", "getArgsTypesList"
        >
    ];
}

def MeasureOpIface : OpInterface<"MeasureOpIface"> {
    let description = [{
        Interface for NetQASM measure operations.
    }];
    let cppNamespace = "qoala::dialects::netqasm::ifaces";
}

def SingleGateOp : OpInterface<"SingleGateOp"> {
    let description = [{
        Interface for NetQASM operations acting on a single gate.
    }];
    let cppNamespace = "qoala::dialects::netqasm::ifaces";
}

def DualGateOp : OpInterface<"DualGateOp"> {
    let description = [{
        Interface for NetQASM operations acting on two gates.
    }];
    let cppNamespace = "qoala::dialects::netqasm::ifaces";
}

def EntangledQubitOp : OpInterface<"EntangledQubitOp"> {
    let description = [{
        Interface for NetQASM operations acting on two gates.
    }];
    let cppNamespace = "qoala::dialects::netqasm::ifaces";
}
#endif // QMEM_IFACE_TD