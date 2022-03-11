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
    %11 = "lir.gate_h"(%2) : (!lir.qubit) -> !lir.qubit
    %12 = "lir.gate_h"(%11) : (!lir.qubit) -> !lir.qubit
    %13 = "lir.gate_h"(%12) : (!lir.qubit) -> !lir.qubit
    %14 = "lir.gate_h"(%13) : (!lir.qubit) -> !lir.qubit
    %15 = "lir.alloc"() : () -> !lir.qubit
    %16 = "lir.gate_h"(%15) : (!lir.qubit) -> !lir.qubit
    %17 = "lir.gate_h"(%16) : (!lir.qubit) -> !lir.qubit
    %18 = "lir.rot_x"(%qout1, %7) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %19 = "lir.rot_x"(%18, %7) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %20 = "lir.rot_x"(%19, %7) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %21 = "lir.rot_x"(%20, %7) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %22 = "lir.rot_z"(%21, %9) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %23 = "lir.meas"(%qout0) : (!lir.qubit) -> !lir.cvalue_q
    %24 = "lir.cval_q_to_c"(%23) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%24) : (!lir.cvalue_c) -> ()
    %25 = "lir.cval_c_to_q"(%10) : (!lir.cvalue_c) -> !lir.cvalue_q
    %26 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %27 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %28 = "lir.cval_c_to_q"(%27) : (!lir.cvalue_c) -> !lir.cvalue_q
    %29 = "lir.rot_z"(%22, %25) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %30 = "lir.meas"(%29) : (!lir.qubit) -> !lir.cvalue_q
    %31 = "lir.rot_z"(%14, %28) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %32 = "lir.gate_h"(%31) : (!lir.qubit) -> !lir.qubit
    %33 = "lir.meas"(%32) : (!lir.qubit) -> !lir.cvalue_q
    return
  }
}

