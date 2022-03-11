module  {
  func @f() {
    %0 = "lir.alloc"() : () -> !lir.qubit
    %1 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %2 = "lir.cval_c_to_q"(%1) : (!lir.cvalue_c) -> !lir.cvalue_q
    %3 = "lir.rot_y"(%0, %2) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %4 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %5 = "lir.cval_c_to_q"(%4) : (!lir.cvalue_c) -> !lir.cvalue_q
    %6 = "lir.rot_z"(%3, %5) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %7 = "lir.entangle"() : () -> !lir.qubit
    %qout0, %qout1 = "lir.cnot"(%6, %7) : (!lir.qubit, !lir.qubit) -> (!lir.qubit, !lir.qubit)
    %8 = "lir.gate_h"(%qout0) : (!lir.qubit) -> !lir.qubit
    %9 = "lir.meas"(%8) : (!lir.qubit) -> !lir.cvalue_q
    %10 = "lir.meas"(%qout1) : (!lir.qubit) -> !lir.cvalue_q
    %11 = "lir.cval_q_to_c"(%9) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%11) : (!lir.cvalue_c) -> ()
    %12 = "lir.cval_q_to_c"(%10) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%12) : (!lir.cvalue_c) -> ()
    return
  }
}

