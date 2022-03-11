prev node = %8 = "lir.new_cval_c"() : () -> !lir.cvalue_c
prev node is classical
prev node = %7 = "lir.gate_h"(%6) : (!lir.qubit) -> !lir.qubit
prev node = %7 = "lir.gate_h"(%6) : (!lir.qubit) -> !lir.qubit
prev node = %7 = "lir.new_cval_c"() : () -> !lir.cvalue_c
prev node is classical
prev node = %6 = "lir.gate_h"(%5) : (!lir.qubit) -> !lir.qubit
prev node = %6 = "lir.gate_h"(%5) : (!lir.qubit) -> !lir.qubit
module  {
  func @f() {
    %0 = "lir.entangle"() : () -> !lir.qubit
    %1 = "lir.entangle"() : () -> !lir.qubit
    %2 = "lir.entangle"() : () -> !lir.qubit
    %qout0, %qout1 = "lir.cphase"(%1, %2) : (!lir.qubit, !lir.qubit) -> (!lir.qubit, !lir.qubit)
    %3 = "lir.gate_h"(%0) : (!lir.qubit) -> !lir.qubit
    %4 = "lir.gate_h"(%3) : (!lir.qubit) -> !lir.qubit
    %5 = "lir.gate_h"(%4) : (!lir.qubit) -> !lir.qubit
    %6 = "lir.gate_h"(%5) : (!lir.qubit) -> !lir.qubit
    %7 = "lir.alloc"() : () -> !lir.qubit
    %8 = "lir.gate_h"(%7) : (!lir.qubit) -> !lir.qubit
    %9 = "lir.gate_h"(%8) : (!lir.qubit) -> !lir.qubit
    %10 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %11 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %12 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %13 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %14 = "lir.cval_c_to_q"(%13) : (!lir.cvalue_c) -> !lir.cvalue_q
    %15 = "lir.rot_x"(%qout1, %14) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %16 = "lir.rot_x"(%15, %14) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %17 = "lir.rot_x"(%16, %14) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %18 = "lir.rot_x"(%17, %14) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %19 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %20 = "lir.cval_c_to_q"(%19) : (!lir.cvalue_c) -> !lir.cvalue_q
    %21 = "lir.rot_z"(%18, %20) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %22 = "lir.meas"(%qout0) : (!lir.qubit) -> !lir.cvalue_q
    %23 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %24 = "lir.cval_q_to_c"(%22) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%24) : (!lir.cvalue_c) -> ()
    %25 = "lir.cval_c_to_q"(%23) : (!lir.cvalue_c) -> !lir.cvalue_q
    %26 = "lir.rot_z"(%21, %25) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %27 = "lir.meas"(%26) : (!lir.qubit) -> !lir.cvalue_q
    %28 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %29 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %30 = "lir.cval_c_to_q"(%29) : (!lir.cvalue_c) -> !lir.cvalue_q
    %31 = "lir.rot_z"(%6, %30) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %32 = "lir.gate_h"(%31) : (!lir.qubit) -> !lir.qubit
    %33 = "lir.meas"(%32) : (!lir.qubit) -> !lir.cvalue_q
    return
  }
}

