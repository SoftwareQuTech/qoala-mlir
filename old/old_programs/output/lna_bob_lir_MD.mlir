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
    %11 = "lir.gate_h"(%10) : (!lir.qubit) -> !lir.qubit
    %12 = "lir.gate_h"(%11) : (!lir.qubit) -> !lir.qubit
    %13 = "lir.gate_h"(%12) : (!lir.qubit) -> !lir.qubit
    %14 = "lir.gate_h"(%13) : (!lir.qubit) -> !lir.qubit
    %15 = "lir.alloc"() : () -> !lir.qubit
    %16 = "lir.gate_h"(%15) : (!lir.qubit) -> !lir.qubit
    %17 = "lir.gate_h"(%16) : (!lir.qubit) -> !lir.qubit
    %18 = "lir.rot_x"(%qout1, %6) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %19 = "lir.rot_x"(%18, %6) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %20 = "lir.rot_x"(%19, %6) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %21 = "lir.rot_x"(%20, %6) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %22 = "lir.rot_z"(%21, %9) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %23 = "lir.meas"(%10) : (!lir.qubit) -> !lir.cvalue_q
    %24 = "lir.cval_q_to_c"(%23) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%24) : (!lir.cvalue_c) -> ()
    %25 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %26 = "lir.cval_c_to_q"(%25) : (!lir.cvalue_c) -> !lir.cvalue_q
    %27 = "lir.rot_z"(%22, %26) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %28 = "lir.meas"(%27) : (!lir.qubit) -> !lir.cvalue_q
    %29 = "lir.cval_q_to_c"(%28) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%29) : (!lir.cvalue_c) -> ()
    return
  }
}

