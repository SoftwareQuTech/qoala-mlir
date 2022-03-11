module  {
  func @f() {
    %0 = "lir.entangle"() : () -> !lir.qubit
    %1 = "lir.entangle"() : () -> !lir.qubit
    %2 = "lir.entangle"() : () -> !lir.qubit
    %3 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %qout0, %qout1 = "lir.cphase"(%1, %2) : (!lir.qubit, !lir.qubit) -> (!lir.qubit, !lir.qubit)
    %4 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %5 = "lir.alloc"() : () -> !lir.qubit
    %6 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %7 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %8 = "lir.cval_c_to_q"(%7) : (!lir.cvalue_c) -> !lir.cvalue_q
    %9 = "lir.add_cval_q"(%8, %8) : (!lir.cvalue_q, !lir.cvalue_q) -> !lir.cvalue_q
    %10 = "lir.add_cval_q"(%8, %9) : (!lir.cvalue_q, !lir.cvalue_q) -> !lir.cvalue_q
    %11 = "lir.add_cval_q"(%8, %10) : (!lir.cvalue_q, !lir.cvalue_q) -> !lir.cvalue_q
    %12 = "lir.rot_x"(%qout1, %11) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %13 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %14 = "lir.cval_c_to_q"(%13) : (!lir.cvalue_c) -> !lir.cvalue_q
    %15 = "lir.meas"(%qout0) : (!lir.qubit) -> !lir.cvalue_q
    %16 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %17 = "lir.cval_q_to_c"(%15) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%17) : (!lir.cvalue_c) -> ()
    %18 = "lir.cval_c_to_q"(%16) : (!lir.cvalue_c) -> !lir.cvalue_q
    %19 = "lir.meas"(%12) : (!lir.qubit) -> !lir.cvalue_q
    %20 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %21 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %22 = "lir.cval_c_to_q"(%21) : (!lir.cvalue_c) -> !lir.cvalue_q
    %23 = "lir.rot_z"(%0, %22) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %24 = "lir.gate_h"(%23) : (!lir.qubit) -> !lir.qubit
    %25 = "lir.meas"(%24) : (!lir.qubit) -> !lir.cvalue_q
    return
  }
}

