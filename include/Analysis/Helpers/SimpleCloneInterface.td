#ifndef QMEM_IFACE_TD
#define QMEM_IFACE_TD

include "mlir/IR/Interfaces.td"

def SimpleCloneInterface : OpInterface<"SimpleCloneInterface"> {
    let description = [{
        Interface for operations that can be cloned in a simple way
    }];
    let cppNamespace = "::qoala::helpers";
    let methods = [
        InterfaceMethod<[{
            Returns a clone of this operation
        }], "::mlir::Operation *", "simpleClone",
        (ins "::mlir::OpBuilder &":$builder, "::mlir::Location":$loc)
        >
    ];
}
#endif // QMEM_IFACE_TD