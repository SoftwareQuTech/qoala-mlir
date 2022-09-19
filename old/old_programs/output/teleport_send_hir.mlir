module  {
  func @f() {
    %0 = "hir.alloc"() : () -> !hir.qubit
    %1 = "hir.new_cval"() : () -> !hir.cvalue
    %2 = "hir.rot_y"(%0, %1) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %3 = "hir.new_cval"() : () -> !hir.cvalue
    %4 = "hir.rot_z"(%2, %3) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %5 = "hir.entangle"() : () -> !hir.qubit
    %qout0, %qout1 = "hir.cnot"(%4, %5) : (!hir.qubit, !hir.qubit) -> (!hir.qubit, !hir.qubit)
    %6 = "hir.gate_h"(%qout0) : (!hir.qubit) -> !hir.qubit
    %7 = "hir.meas"(%6) : (!hir.qubit) -> !hir.cvalue
    "hir.send_cmsg"(%7) : (!hir.cvalue) -> ()
    %8 = "hir.meas"(%qout1) : (!hir.qubit) -> !hir.cvalue
    "hir.send_cmsg"(%8) : (!hir.cvalue) -> ()
    return
  }
}

