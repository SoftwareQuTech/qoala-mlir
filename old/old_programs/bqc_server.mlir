func.func @f() -> () {
    %q0 = "hir.entangle"() : () -> !hir.qubit
    %q1 = "hir.entangle"() : () -> !hir.qubit
    %q0a, %q1a = "hir.cphase"(%q0, %q1) : (!hir.qubit, !hir.qubit) -> (!hir.qubit, !hir.qubit)
    %d0 = "hir.recv_cmsg"() : () -> !hir.cvalue
    %q0b = "hir.rot_z"(%q0a, %d0) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %q0c = "hir.gate_h"(%q0b) : (!hir.qubit)-> !hir.qubit
    %m0 = "hir.meas"(%q0c) : (!hir.qubit) -> !hir.cvalue
    "hir.send_cmsg"(%m0) : (!hir.cvalue) -> ()
    %d1 = "hir.recv_cmsg"() : () -> !hir.cvalue
    %q1b = "hir.rot_z"(%q1a, %d1) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %q1c = "hir.gate_h"(%q1b) : (!hir.qubit)-> !hir.qubit
    %m1 = "hir.meas"(%q1c) : (!hir.qubit) -> !hir.cvalue
    "hir.send_cmsg"(%m1) : (!hir.cvalue) -> ()
    return
}