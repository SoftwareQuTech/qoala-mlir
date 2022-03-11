module  {
  func @f() {
    %0 = "lir.entangle"() : () -> !lir.qubit
    %1 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %2 = "lir.cval_c_to_q"(%1) : (!lir.cvalue_c) -> !lir.cvalue_q
    %3 = "lir.rot_z"(%0, %2) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %4 = "lir.gate_h"(%3) : (!lir.qubit) -> !lir.qubit
    %5 = "lir.entangle"() : () -> !lir.qubit
    %6 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %7 = "lir.cval_c_to_q"(%6) : (!lir.cvalue_c) -> !lir.cvalue_q
    %8 = "lir.rot_z"(%5, %7) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %9 = "lir.gate_h"(%8) : (!lir.qubit) -> !lir.qubit
    %10 = "lir.meas"(%4) : (!lir.qubit) -> !lir.cvalue_q
    %11 = "lir.meas"(%9) : (!lir.qubit) -> !lir.cvalue_q
    %12 = "lir.cval_q_to_c"(%10) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%12) : (!lir.cvalue_c) -> ()
    %13 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %14 = "lir.cval_q_to_c"(%11) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%14) : (!lir.cvalue_c) -> ()
    return
  }
}

