func @f() -> () {
    %q0 = "hir.entangle"() : () -> !hir.qubit
    %q1 = "hir.entangle"() : () -> !hir.qubit
    %q0a, %q1a = "hir.cphase"(%q0, %q1) : (!hir.qubit, !hir.qubit) -> (!hir.qubit, !hir.qubit)
    %d0 = "hir.recv_cmsg"() : () -> !hir.cvalue
    %q0b = "hir.gate_z"(%q0a, %d0) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %m0 = "hir.meas"(%q0a) : (!hir.qubit) -> !hir.cvalue
    "hir.send_cmsg"(%m0) : (!hir.cvalue) -> ()
    %d1 = "hir.recv_cmsg"() : () -> !hir.cvalue
    %q1b = "hir.gate_z"(%q1a, %d1) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %m1 = "hir.meas"(%q1b) : (!hir.qubit) -> !hir.cvalue
    "hir.send_cmsg"(%m1) : (!hir.cvalue) -> ()
    return
}