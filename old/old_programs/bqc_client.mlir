func @f() -> () {
    %theta0 = "hir.new_cval"() : () -> !hir.cvalue
    %q0 = "hir.entangle"() : () -> !hir.qubit
    %q0a = "hir.rot_z"(%q0, %theta0) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %q0b = "hir.gate_h"(%q0a) : (!hir.qubit) -> !hir.qubit

    %theta1 = "hir.new_cval"() : () -> !hir.cvalue
    %q1 = "hir.entangle"() : () -> !hir.qubit
    %q1a = "hir.rot_z"(%q1, %theta1) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %q1b = "hir.gate_h"(%q1a) : (!hir.qubit) -> !hir.qubit

    %m0 = "hir.meas"(%q0b) : (!hir.qubit) -> !hir.cvalue
    "hir.send_cmsg"(%m0) : (!hir.cvalue) -> ()

    %m1 = "hir.meas"(%q1b) : (!hir.qubit) -> !hir.cvalue
    %server_m1 = "hir.recv_cmsg"() : () -> (!hir.cvalue)
    "hir.send_cmsg"(%m1) : (!hir.cvalue) -> ()
    return
}