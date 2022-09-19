func @f() -> () {
    %comm0 = "hir.entangle"() : () -> !hir.qubit
    %mem0 = "hir.alloc"() : () -> !hir.qubit
    %mem1 = "hir.gate_h"(%mem0) : (!hir.qubit) -> !hir.qubit
    %comm1, %mem2 = "hir.cphase"(%comm0, %mem1) : (!hir.qubit, !hir.qubit) -> (!hir.qubit, !hir.qubit)

    %d1 = "hir.recv_cmsg"() : () -> !hir.cvalue
    %comm2 = "hir.rot_z"(%comm1, %d1) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %comm3 = "hir.gate_h"(%comm2) : (!hir.qubit) -> !hir.qubit
    %m1 = "hir.meas"(%comm3) : (!hir.qubit) -> !hir.cvalue

    "hir.send_cmsg"(%m1) : (!hir.cvalue) -> ()

    %d2 = "hir.recv_cmsg"() : () -> !hir.cvalue
    %mem3 = "hir.rot_z"(%mem2, %d2) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %mem4 = "hir.gate_h"(%mem3) : (!hir.qubit) -> !hir.qubit
    %m2 = "hir.meas"(%mem4) : (!hir.qubit) -> !hir.cvalue

    "hir.send_cmsg"(%m2) : (!hir.cvalue) -> ()
    return
}