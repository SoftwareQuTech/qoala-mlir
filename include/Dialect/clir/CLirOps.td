#ifndef CLIR_OPS
#define CLIR_OPS

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

include "Dialect/clir/CLirDialect.td"
include "Dialect/clir/CLirTypes.td"

def CLir_SendCMsgOp : CLir_Op<"send_cmsg"> {
    let arguments = (ins CLir_CMsgType:$cmout);

    let verifier = ?;
}

def CLir_RecvCMsgOp : CLir_Op<"recv_cmsg"> {
    let results = (outs CLir_CMsgType:$cmin);

    let verifier = ?;
}

def CLir_RecvQLirValueOp : CLir_Op<"recv_qlir"> {
    let arguments = (outs CLir_ValueType:$val);
    let verifier = ?;
}

def CLir_SendQLirValueOp : CLir_Op<"send_qlir"> {
    let arguments = (ins CLir_ValueType:$val);
    let verifier = ?;
}



#endif // CLIR_OPS
