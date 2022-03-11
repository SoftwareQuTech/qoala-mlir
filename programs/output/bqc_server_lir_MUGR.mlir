module  {
  func @f() {
    %0 = "lir.entangle"() : () -> !lir.qubit
    %1 = "lir.entangle"() : () -> !lir.qubit
    %qout0, %qout1 = "lir.cphase"(%0, %1) : (!lir.qubit, !lir.qubit) -> (!lir.qubit, !lir.qubit)
    %2 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %3 = "lir.cval_c_to_q"(%2) : (!lir.cvalue_c) -> !lir.cvalue_q
    %4 = "lir.rot_z"(%qout0, %3) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %5 = "lir.gate_h"(%4) : (!lir.qubit) -> !lir.qubit
    %6 = "lir.meas"(%5) : (!lir.qubit) -> !lir.cvalue_q
    %7 = "lir.cval_q_to_c"(%6) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%7) : (!lir.cvalue_c) -> ()
    %8 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %9 = "lir.cval_c_to_q"(%8) : (!lir.cvalue_c) -> !lir.cvalue_q
    %10 = "lir.rot_z"(%qout1, %9) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %11 = "lir.gate_h"(%10) : (!lir.qubit) -> !lir.qubit
    %12 = "lir.meas"(%11) : (!lir.qubit) -> !lir.cvalue_q
    %13 = "lir.cval_q_to_c"(%12) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%13) : (!lir.cvalue_c) -> ()
    return
  }
}

