module  {
  func @f() {
    %0 = "hir.entangle"() : () -> !hir.qubit
    %1 = "hir.entangle"() : () -> !hir.qubit
    %2 = "hir.entangle"() : () -> !hir.qubit
    %3 = "hir.new_cval"() : () -> !hir.cvalue
    %qout0, %qout1 = "hir.cphase"(%1, %2) : (!hir.qubit, !hir.qubit) -> (!hir.qubit, !hir.qubit)
    %4 = "hir.gate_h"(%qout0) : (!hir.qubit) -> !hir.qubit
    %5 = "hir.gate_h"(%4) : (!hir.qubit) -> !hir.qubit
    %6 = "hir.gate_h"(%5) : (!hir.qubit) -> !hir.qubit
    %7 = "hir.gate_h"(%6) : (!hir.qubit) -> !hir.qubit
    %8 = "hir.gate_h"(%7) : (!hir.qubit) -> !hir.qubit
    %9 = "hir.alloc"() : () -> !hir.qubit
    %10 = "hir.gate_h"(%9) : (!hir.qubit) -> !hir.qubit
    %11 = "hir.gate_h"(%10) : (!hir.qubit) -> !hir.qubit
    %12 = "hir.new_cval"() : () -> !hir.cvalue
    "hir.send_cmsg"(%12) : (!hir.cvalue) -> ()
    %13 = "hir.new_cval"() : () -> !hir.cvalue
    %14 = "hir.rot_x"(%qout1, %13) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %15 = "hir.rot_x"(%14, %13) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %16 = "hir.new_cval"() : () -> !hir.cvalue
    %17 = "hir.rot_x"(%15, %13) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %18 = "hir.rot_x"(%17, %13) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %19 = "hir.new_cval"() : () -> !hir.cvalue
    "hir.send_cmsg"(%19) : (!hir.cvalue) -> ()
    %20 = "hir.rot_z"(%18, %19) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %21 = "hir.meas"(%4) : (!hir.qubit) -> !hir.cvalue
    "hir.send_cmsg"(%21) : (!hir.cvalue) -> ()
    %22 = "hir.recv_cmsg"() : () -> !hir.cvalue
    %23 = "hir.rot_z"(%20, %22) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %24 = "hir.meas"(%23) : (!hir.qubit) -> !hir.cvalue
    "hir.send_cmsg"(%24) : (!hir.cvalue) -> ()
    return
  }
}

