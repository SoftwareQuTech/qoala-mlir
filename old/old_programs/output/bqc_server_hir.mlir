module  {
  func @f() {
    %0 = "hir.entangle"() : () -> !hir.qubit
    %1 = "hir.entangle"() : () -> !hir.qubit
    %qout0, %qout1 = "hir.cphase"(%0, %1) : (!hir.qubit, !hir.qubit) -> (!hir.qubit, !hir.qubit)
    %2 = "hir.recv_cmsg"() : () -> !hir.cvalue
    %3 = "hir.rot_z"(%qout0, %2) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %4 = "hir.gate_h"(%3) : (!hir.qubit) -> !hir.qubit
    %5 = "hir.meas"(%4) : (!hir.qubit) -> !hir.cvalue
    "hir.send_cmsg"(%5) : (!hir.cvalue) -> ()
    %6 = "hir.recv_cmsg"() : () -> !hir.cvalue
    %7 = "hir.rot_z"(%qout1, %6) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %8 = "hir.gate_h"(%7) : (!hir.qubit) -> !hir.qubit
    %9 = "hir.meas"(%8) : (!hir.qubit) -> !hir.cvalue
    "hir.send_cmsg"(%9) : (!hir.cvalue) -> ()
    return
  }
}

