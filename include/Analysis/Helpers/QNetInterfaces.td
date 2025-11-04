#ifndef QNET_IFACE_TD
#define QNET_IFACE_TD

include "mlir/IR/Interfaces.td"

def HermitianOpIface : OpInterface<"HermitianOpIface"> {
    let description = [{
        Interface for hermitian operations.
    }];
    let cppNamespace = "::qoala::helpers";
    let methods = [];
}

#endif // QNET_IFACE_TD