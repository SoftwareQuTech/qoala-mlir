module  {
  func @f() {
    %0 = "hir.entangle"() : () -> !hir.qubit
    %1 = "hir.entangle"() : () -> !hir.qubit
    %2 = "hir.entangle"() : () -> !hir.qubit
    %3 = "hir.new_cval"() : () -> !hir.cvalue
    %qout0, %qout1 = "hir.cphase"(%1, %2) : (!hir.qubit, !hir.qubit) -> (!hir.qubit, !hir.qubit)
    %4 = "hir.gate_h"(%0) : (!hir.qubit) -> !hir.qubit
    %5 = "hir.gate_h"(%4) : (!hir.qubit) -> !hir.qubit
    %6 = "hir.gate_h"(%5) : (!hir.qubit) -> !hir.qubit
    %7 = "hir.gate_h"(%6) : (!hir.qubit) -> !hir.qubit
    %8 = "hir.new_cval"() : () -> !hir.cvalue
    %9 = "hir.alloc"() : () -> !hir.qubit
    %10 = "hir.gate_h"(%9) : (!hir.qubit) -> !hir.qubit
    %11 = "hir.gate_h"(%10) : (!hir.qubit) -> !hir.qubit
    %12 = "hir.recv_cmsg"() : () -> !hir.cvalue
    %13 = "hir.new_cval"() : () -> !hir.cvalue
    %14 = "hir.rot_x"(%qout1, %13) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %15 = "hir.rot_x"(%14, %13) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %16 = "hir.rot_x"(%15, %13) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %17 = "hir.rot_x"(%16, %13) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %18 = "hir.recv_cmsg"() : () -> !hir.cvalue
    %19 = "hir.rot_z"(%17, %18) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %20 = "hir.meas"(%qout0) : (!hir.qubit) -> !hir.cvalue
    %21 = "hir.recv_cmsg"() : () -> !hir.cvalue
    "hir.send_cmsg"(%20) : (!hir.cvalue) -> ()
    %22 = "hir.rot_z"(%19, %21) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %23 = "hir.meas"(%22) : (!hir.qubit) -> !hir.cvalue
    %24 = "hir.recv_cmsg"() : () -> !hir.cvalue
    %25 = "hir.new_cval"() : () -> !hir.cvalue
    %26 = "hir.rot_z"(%7, %25) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %27 = "hir.gate_h"(%26) : (!hir.qubit) -> !hir.qubit
    %28 = "hir.meas"(%27) : (!hir.qubit) -> !hir.cvalue
    return
  }
}

