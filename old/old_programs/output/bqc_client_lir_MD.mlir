module  {
  func @f() {
    %0 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %1 = "lir.cval_c_to_q"(%0) : (!lir.cvalue_c) -> !lir.cvalue_q
    %2 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %3 = "lir.cval_c_to_q"(%2) : (!lir.cvalue_c) -> !lir.cvalue_q
    %4 = "lir.entangle"() : () -> !lir.qubit
    %5 = "lir.rot_z"(%4, %1) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %6 = "lir.gate_h"(%5) : (!lir.qubit) -> !lir.qubit
    %7 = "lir.entangle"() : () -> !lir.qubit
    %8 = "lir.rot_z"(%7, %3) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %9 = "lir.gate_h"(%8) : (!lir.qubit) -> !lir.qubit
    %10 = "lir.meas"(%6) : (!lir.qubit) -> !lir.cvalue_q
    %11 = "lir.cval_q_to_c"(%10) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%11) : (!lir.cvalue_c) -> ()
    %12 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %13 = "lir.meas"(%9) : (!lir.qubit) -> !lir.cvalue_q
    %14 = "lir.cval_q_to_c"(%13) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%14) : (!lir.cvalue_c) -> ()
    return
  }
}

