#ifndef GENERIC_IFACES_TD
#define GENERIC_IFACES_TD

include "mlir/IR/Interfaces.td"

def OpHasDurationInterface : OpInterface<"OpHasDurationInterface"> {
    let description = [{
        Interface for NetQASM and QoalaHost operations that have a duration value.
    }];
    let cppNamespace = "::qoala::helpers";
    let methods = [
        InterfaceMethod<[{
            Returns the return operation of this routine
        }], "int", "getDuration", (ins), /*methodBody=*/[{}],
        /*defaultImplementation=*/[{
            return qoala::options::qoalaOptHostInstrTime;
        }]
        >
    ];
}
#endif // GENERIC_IFACES_TD