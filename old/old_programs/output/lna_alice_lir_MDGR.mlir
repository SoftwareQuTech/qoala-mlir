module  {
  func @f() {
    %0 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %1 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %2 = "lir.entangle"() : () -> !lir.qubit
    %3 = "lir.entangle"() : () -> !lir.qubit
    %4 = "lir.entangle"() : () -> !lir.qubit
    %5 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %6 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %7 = "lir.cval_c_to_q"(%6) : (!lir.cvalue_c) -> !lir.cvalue_q
    %8 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %9 = "lir.cval_c_to_q"(%8) : (!lir.cvalue_c) -> !lir.cvalue_q
    %10 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %qout0, %qout1 = "lir.cphase"(%3, %4) : (!lir.qubit, !lir.qubit) -> (!lir.qubit, !lir.qubit)
    %11 = "lir.alloc"() : () -> !lir.qubit
    %12 = "lir.add_cval_q"(%7, %7) : (!lir.cvalue_q, !lir.cvalue_q) -> !lir.cvalue_q
    %13 = "lir.add_cval_q"(%7, %12) : (!lir.cvalue_q, !lir.cvalue_q) -> !lir.cvalue_q
    %14 = "lir.add_cval_q"(%7, %13) : (!lir.cvalue_q, !lir.cvalue_q) -> !lir.cvalue_q
    %15 = "lir.rot_x"(%qout1, %14) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %16 = "lir.meas"(%qout0) : (!lir.qubit) -> !lir.cvalue_q
    %17 = "lir.cval_q_to_c"(%16) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%17) : (!lir.cvalue_c) -> ()
    %18 = "lir.cval_c_to_q"(%10) : (!lir.cvalue_c) -> !lir.cvalue_q
    %19 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %20 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %21 = "lir.cval_c_to_q"(%20) : (!lir.cvalue_c) -> !lir.cvalue_q
    %22 = "lir.meas"(%15) : (!lir.qubit) -> !lir.cvalue_q
    %23 = "lir.rot_z"(%2, %21) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %24 = "lir.gate_h"(%23) : (!lir.qubit) -> !lir.qubit
    %25 = "lir.meas"(%24) : (!lir.qubit) -> !lir.cvalue_q
    return
  }
}

