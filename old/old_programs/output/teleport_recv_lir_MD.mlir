module  {
  func @f() {
    %0 = "lir.entangle"() : () -> !lir.qubit
    %1 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %2 = "lir.cval_c_to_q"(%1) : (!lir.cvalue_c) -> !lir.cvalue_q
    %3 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %4 = "lir.cval_c_to_q"(%3) : (!lir.cvalue_c) -> !lir.cvalue_q
    %5 = "lir.rot_x"(%0, %2) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %6 = "lir.rot_z"(%5, %4) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %7 = "lir.meas"(%6) : (!lir.qubit) -> !lir.cvalue_q
    return
  }
}

