func @f() -> () {
    %epr0 = "hir.entangle"() : () -> !hir.qubit

    %m1 = "hir.recv_cmsg"() : () -> (!hir.cvalue)
    %epr1 = "hir.rot_x"(%epr0, %m1) : (!hir.qubit, !hir.cvalue) -> !hir.qubit

    %m2 = "hir.recv_cmsg"() : () -> (!hir.cvalue)
    %epr2 = "hir.rot_z"(%epr1, %m2) : (!hir.qubit, !hir.cvalue) -> !hir.qubit

    %result = "hir.meas"(%epr2) : (!hir.qubit) -> !hir.cvalue

    return
}