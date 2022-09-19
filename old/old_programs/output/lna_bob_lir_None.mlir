module  {
  func @f() {
    %0 = "lir.entangle"() : () -> !lir.qubit
    %1 = "lir.entangle"() : () -> !lir.qubit
    %2 = "lir.entangle"() : () -> !lir.qubit
    %3 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %qout0, %qout1 = "lir.cphase"(%1, %2) : (!lir.qubit, !lir.qubit) -> (!lir.qubit, !lir.qubit)
    %4 = "lir.gate_h"(%qout0) : (!lir.qubit) -> !lir.qubit
    %5 = "lir.gate_h"(%4) : (!lir.qubit) -> !lir.qubit
    %6 = "lir.gate_h"(%5) : (!lir.qubit) -> !lir.qubit
    %7 = "lir.gate_h"(%6) : (!lir.qubit) -> !lir.qubit
    %8 = "lir.gate_h"(%7) : (!lir.qubit) -> !lir.qubit
    %9 = "lir.alloc"() : () -> !lir.qubit
    %10 = "lir.gate_h"(%9) : (!lir.qubit) -> !lir.qubit
    %11 = "lir.gate_h"(%10) : (!lir.qubit) -> !lir.qubit
    %12 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    "lir.send_cmsg"(%12) : (!lir.cvalue_c) -> ()
    %13 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %14 = "lir.cval_c_to_q"(%13) : (!lir.cvalue_c) -> !lir.cvalue_q
    %15 = "lir.rot_x"(%qout1, %14) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %16 = "lir.rot_x"(%15, %14) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %17 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %18 = "lir.rot_x"(%16, %14) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %19 = "lir.rot_x"(%18, %14) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %20 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    "lir.send_cmsg"(%20) : (!lir.cvalue_c) -> ()
    %21 = "lir.cval_c_to_q"(%20) : (!lir.cvalue_c) -> !lir.cvalue_q
    %22 = "lir.rot_z"(%19, %21) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %23 = "lir.meas"(%4) : (!lir.qubit) -> !lir.cvalue_q
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

