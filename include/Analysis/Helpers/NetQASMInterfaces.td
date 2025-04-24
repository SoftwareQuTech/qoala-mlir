#ifndef QMEM_IFACE_TD
#define QMEM_IFACE_TD

include "mlir/IR/Interfaces.td"

def NetQASMRoutineInterface : OpInterface<"NetQASMRoutineInterface"> {
    let description = [{
        Interface for NetQASM operations that contain a region.
    }];
    let cppNamespace = "::qoala::helpers";
    let methods = [
        InterfaceMethod<[{
            Returns the return operation of this routine
        }], "::mlir::Operation *", "getReturnOperation"
        >
    ];
}
#endif // QMEM_IFACE_TD