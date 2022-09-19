module  {
  func @f() {
    %0 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %1 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %2 = "lir.entangle"() : () -> !lir.qubit
    %3 = "lir.entangle"() : () -> !lir.qubit
    %4 = "lir.entangle"() : () -> !lir.qubit
    "lir.send_cmsg"(%1) : (!lir.cvalue_c) -> ()
    %5 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %6 = "lir.cval_c_to_q"(%5) : (!lir.cvalue_c) -> !lir.cvalue_q
    %7 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %8 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    "lir.send_cmsg"(%8) : (!lir.cvalue_c) -> ()
    %9 = "lir.cval_c_to_q"(%8) : (!lir.cvalue_c) -> !lir.cvalue_q
    %qout0, %qout1 = "lir.cphase"(%3, %4) : (!lir.qubit, !lir.qubit) -> (!lir.qubit, !lir.qubit)
    %10 = "lir.gate_h"(%qout0) : (!lir.qubit) -> !lir.qubit
    %11 = "lir.alloc"() : () -> !lir.qubit
    %12 = "lir.add_cval_q"(%6, %6) : (!lir.cvalue_q, !lir.cvalue_q) -> !lir.cvalue_q
    %13 = "lir.add_cval_q"(%6, %12) : (!lir.cvalue_q, !lir.cvalue_q) -> !lir.cvalue_q
    %14 = "lir.add_cval_q"(%6, %13) : (!lir.cvalue_q, !lir.cvalue_q) -> !lir.cvalue_q
    %15 = "lir.rot_x"(%qout1, %14) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %16 = "lir.meas"(%10) : (!lir.qubit) -> !lir.cvalue_q
    %17 = "lir.cval_q_to_c"(%16) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%17) : (!lir.cvalue_c) -> ()
    %18 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %19 = "lir.cval_c_to_q"(%18) : (!lir.cvalue_c) -> !lir.cvalue_q
    %20 = "lir.meas"(%15) : (!lir.qubit) -> !lir.cvalue_q
    %21 = "lir.cval_q_to_c"(%20) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%21) : (!lir.cvalue_c) -> ()
    return
  }
}

