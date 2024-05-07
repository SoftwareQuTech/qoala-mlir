#ifndef QREMOTE_OPS_TD
#define QREMOTE_OPS_TD

include "mlir/IR/OpBase.td"
include "mlir/IR/SymbolInterfaces.td"
include "Dialect/QoalaHost/QoalaHostDialect.td"

class QRemote_Op<string mnemonic, list<Trait> traits = []> :
    Op<QRemote_Dialect, mnemonic, traits>;


def RemoteOp: QRemote_Op<"remote", [Symbol]> {
    let arguments = (ins Arg<SymbolNameAttr, "remote-name">:$sym_name,
                         Arg<OptionalAttr<StrAttr>, "visibility">:$sym_visibility);
    let assemblyFormat = [{
        attr-dict $sym_name
    }];
}

#endif // QREMOTE_OPS_TD