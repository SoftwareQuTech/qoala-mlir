func @f() -> () {
    %q0 = "hir.entangle"() : () -> !hir.qubit
    %q1 = "hir.entangle"() : () -> !hir.qubit
    %m0 = "hir.meas"(%q0) : (!hir.qubit) -> !hir.cvalue
    %m1 = "hir.meas"(%q1) : (!hir.qubit) -> !hir.cvalue
    "hir.send_cmsg"(%m0) : (!hir.cvalue) -> ()
    "hir.send_cmsg"(%m1) : (!hir.cvalue) -> ()
    return
}