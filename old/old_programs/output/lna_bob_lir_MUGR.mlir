prev node = %8 = "lir.gate_h"(%7) : (!lir.qubit) -> !lir.qubit
prev node = %8 = "lir.new_cval_c"() : () -> !lir.cvalue_c
prev node is classical
prev node = %7 = "lir.gate_h"(%6) : (!lir.qubit) -> !lir.qubit
prev node = %7 = "lir.gate_h"(%6) : (!lir.qubit) -> !lir.qubit
module  {
  func @f() {
    %0 = "lir.entangle"() : () -> !lir.qubit
    %1 = "lir.entangle"() : () -> !lir.qubit
    %2 = "lir.entangle"() : () -> !lir.qubit
    %qout0, %qout1 = "lir.cphase"(%1, %2) : (!lir.qubit, !lir.qubit) -> (!lir.qubit, !lir.qubit)
    %3 = "lir.gate_h"(%qout0) : (!lir.qubit) -> !lir.qubit
    %4 = "lir.alloc"() : () -> !lir.qubit
    %5 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %6 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    "lir.send_cmsg"(%6) : (!lir.cvalue_c) -> ()
    %7 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %8 = "lir.cval_c_to_q"(%7) : (!lir.cvalue_c) -> !lir.cvalue_q
    %9 = "lir.add_cval_q"(%8, %8) : (!lir.cvalue_q, !lir.cvalue_q) -> !lir.cvalue_q
    %10 = "lir.add_cval_q"(%8, %9) : (!lir.cvalue_q, !lir.cvalue_q) -> !lir.cvalue_q
    %11 = "lir.add_cval_q"(%8, %10) : (!lir.cvalue_q, !lir.cvalue_q) -> !lir.cvalue_q
    %12 = "lir.rot_x"(%qout1, %11) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %13 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %14 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    "lir.send_cmsg"(%14) : (!lir.cvalue_c) -> ()
    %15 = "lir.cval_c_to_q"(%14) : (!lir.cvalue_c) -> !lir.cvalue_q
    %16 = "lir.meas"(%3) : (!lir.qubit) -> !lir.cvalue_q
    %17 = "lir.cval_q_to_c"(%16) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%17) : (!lir.cvalue_c) -> ()
    %18 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %19 = "lir.cval_c_to_q"(%18) : (!lir.cvalue_c) -> !lir.cvalue_q
    %20 = "lir.meas"(%12) : (!lir.qubit) -> !lir.cvalue_q
    %21 = "lir.cval_q_to_c"(%20) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%21) : (!lir.cvalue_c) -> ()
    return
  }
}

