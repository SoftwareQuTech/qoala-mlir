module  {
  func @f() {
    %0 = "hir.new_cval"() : () -> !hir.cvalue
    %1 = "hir.entangle"() : () -> !hir.qubit
    %2 = "hir.rot_z"(%1, %0) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %3 = "hir.gate_h"(%2) : (!hir.qubit) -> !hir.qubit
    %4 = "hir.new_cval"() : () -> !hir.cvalue
    %5 = "hir.entangle"() : () -> !hir.qubit
    %6 = "hir.rot_z"(%5, %4) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %7 = "hir.gate_h"(%6) : (!hir.qubit) -> !hir.qubit
    %8 = "hir.meas"(%3) : (!hir.qubit) -> !hir.cvalue
    "hir.send_cmsg"(%8) : (!hir.cvalue) -> ()
    %9 = "hir.meas"(%7) : (!hir.qubit) -> !hir.cvalue
    %10 = "hir.recv_cmsg"() : () -> !hir.cvalue
    "hir.send_cmsg"(%9) : (!hir.cvalue) -> ()
    return
  }
}

