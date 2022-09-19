module  {
  func @f() {
    %0 = "hir.entangle"() : () -> !hir.qubit
    %1 = "hir.recv_cmsg"() : () -> !hir.cvalue
    %2 = "hir.rot_x"(%0, %1) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %3 = "hir.recv_cmsg"() : () -> !hir.cvalue
    %4 = "hir.rot_z"(%2, %3) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %5 = "hir.meas"(%4) : (!hir.qubit) -> !hir.cvalue
    return
  }
}

