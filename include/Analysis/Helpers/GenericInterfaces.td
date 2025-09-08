#ifndef GENERIC_IFACES_TD
#define GENERIC_IFACES_TD

include "mlir/IR/Interfaces.td"

def QuantumOpInterface : OpInterface<"QuantumOpInterface"> {
    let description = [{
        Interface for NetQASM and QoalaHost operations to have common methods.
    }];
    let cppNamespace = "qoala::helpers";
    let methods = [
        InterfaceMethod<[{
            Returns the duration of this operation.
        }], "uint32_t", "getDuration", (ins), /*methodBody=*/[{}],
        /*defaultImplementation=*/[{
            return qoala::options::qoalaOptHostInstrTime;
        }]
        >,
        InterfaceMethod<[{
            Returns the block type where this operation belongs to.
        }], "qoala::analysis::reordering::BlockType", "getBlockType",
        (ins "const llvm::StringMap<mlir::Operation *> &":$routineMap),
        /*methodBody=*/[{}],
        /*defaultImplementation=*/[{
            return qoala::analysis::reordering::BlockType::CL;
        }]
        >
    ];
}

def ClassicaCommInterface : OpInterface<"ClassicaCommInterface"> {
    let description = [{
        Interface for QoalaHost operations representing classical communication.
    }];
    let cppNamespace = "qoala::helpers";
}
#endif // GENERIC_IFACES_TD