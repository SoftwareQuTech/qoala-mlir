#ifndef QOALAHOST_IFACES_TD
#define QOALAHOST_IFACES_TD

include "mlir/IR/Interfaces.td"

def ClassicalCommInterface : OpInterface<"ClassicalCommInterface"> {
    let description = [{
        Interface for QoalaHost operations representing classical communication.
    }];
    let cppNamespace = "qoala::dialects::qoalahost::ifaces";
}
#endif // QOALAHOST_IFACES_TD
