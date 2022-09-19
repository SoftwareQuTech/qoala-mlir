module  {
  func @f() {
    %0 = "lir.entangle"() : () -> !lir.qubit
    %1 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %2 = "lir.cval_c_to_q"(%1) : (!lir.cvalue_c) -> !lir.cvalue_q
    %3 = "lir.rot_x"(%0, %2) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %4 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %5 = "lir.cval_c_to_q"(%4) : (!lir.cvalue_c) -> !lir.cvalue_q
    %6 = "lir.rot_z"(%3, %5) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %7 = "lir.meas"(%6) : (!lir.qubit) -> !lir.cvalue_q
    return
  }
}

