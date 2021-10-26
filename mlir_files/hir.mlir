func @f() -> () {
    %q0 = "hir.alloc"() : () -> !hir.qubit
    %c0 = "hir.recv_cmsg"() : () -> !hir.cvalue
    %q1 = "hir.gate_x"(%q0, %c0) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %q2 = "hir.gate_x"(%q1, %c0) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %m = "hir.meas"(%q2) : (!hir.qubit) -> !hir.cvalue
    "hir.send_cmsg"(%m) : (!hir.cvalue) -> ()
    return
}