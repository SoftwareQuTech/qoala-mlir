module  {
  func @f() {
    %0 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %1 = "lir.cval_c_to_q"(%0) : (!lir.cvalue_c) -> !lir.cvalue_q
    %2 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %3 = "lir.cval_c_to_q"(%2) : (!lir.cvalue_c) -> !lir.cvalue_q
    %4 = "lir.alloc"() : () -> !lir.qubit
    %5 = "lir.rot_y"(%4, %1) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %6 = "lir.rot_z"(%5, %3) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %7 = "lir.entangle"() : () -> !lir.qubit
    %qout0, %qout1 = "lir.cnot"(%6, %7) : (!lir.qubit, !lir.qubit) -> (!lir.qubit, !lir.qubit)
    %8 = "lir.gate_h"(%qout0) : (!lir.qubit) -> !lir.qubit
    %9 = "lir.meas"(%8) : (!lir.qubit) -> !lir.cvalue_q
    %10 = "lir.cval_q_to_c"(%9) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%10) : (!lir.cvalue_c) -> ()
    %11 = "lir.meas"(%qout1) : (!lir.qubit) -> !lir.cvalue_q
    %12 = "lir.cval_q_to_c"(%11) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%12) : (!lir.cvalue_c) -> ()
    return
  }
}

