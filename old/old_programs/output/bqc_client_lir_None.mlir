module  {
  func @f() {
    %0 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %1 = "lir.entangle"() : () -> !lir.qubit
    %2 = "lir.cval_c_to_q"(%0) : (!lir.cvalue_c) -> !lir.cvalue_q
    %3 = "lir.rot_z"(%1, %2) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %4 = "lir.gate_h"(%3) : (!lir.qubit) -> !lir.qubit
    %5 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %6 = "lir.entangle"() : () -> !lir.qubit
    %7 = "lir.cval_c_to_q"(%5) : (!lir.cvalue_c) -> !lir.cvalue_q
    %8 = "lir.rot_z"(%6, %7) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %9 = "lir.gate_h"(%8) : (!lir.qubit) -> !lir.qubit
    %10 = "lir.meas"(%4) : (!lir.qubit) -> !lir.cvalue_q
    %11 = "lir.cval_q_to_c"(%10) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%11) : (!lir.cvalue_c) -> ()
    %12 = "lir.meas"(%9) : (!lir.qubit) -> !lir.cvalue_q
    %13 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %14 = "lir.cval_q_to_c"(%12) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%14) : (!lir.cvalue_c) -> ()
    return
  }
}

