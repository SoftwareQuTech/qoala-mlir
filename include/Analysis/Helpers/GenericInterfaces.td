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
        }], "uint64_t", "getDuration", (ins), /*methodBody=*/[{}],
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

def UsesRemoteInterface : OpInterface<"UsesRemoteInterface"> {
    let description = [{
        Interface for QMem, QoalaHost and NetQASM operations that use a remote reference.
    }];
    let cppNamespace = "qoala::helpers";
    let methods = [
        InterfaceMethod<[{
            Returns a StringRef with the name of the used remote.
        }], "llvm::StringRef", "getUsedRemoteName", (ins),
        /*methodBody=*/[{}],
        /*defaultImplementation=*/[{ return $_op.getRemote(); }]
        >,
        InterfaceMethod<[{
            Returns remote attribute used by this operation
        }], "mlir::FlatSymbolRefAttr", "getUsedRemoteAttr", (ins),
        /*methodBody=*/[{}],
        /*defaultImplementation=*/[{ return $_op.getRemoteAttr(); }]
        >
    ];
}
#endif // GENERIC_IFACES_TD
