func @f() -> () {
    // prepare local qubit
    %q0 = "hir.alloc"() : () -> !hir.qubit
    %theta = "hir.new_cval"() : () -> !hir.cvalue
    %phi = "hir.new_cval"() : () -> !hir.cvalue
    %q1 = "hir.rot_y"(%q0, %theta) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %q2 = "hir.rot_z"(%q1, %phi) : (!hir.qubit, !hir.cvalue) -> !hir.qubit

    // entanglement generation
    %epr0 = "hir.entangle"() : () -> !hir.qubit

    %q3, %epr1 = "hir.cnot"(%q2, %epr0) : (!hir.qubit, !hir.qubit) -> (!hir.qubit, !hir.qubit)
    %q4 = "hir.gate_h"(%q3) : (!hir.qubit) -> !hir.qubit

    %m0 = "hir.meas"(%q4) : (!hir.qubit) -> !hir.cvalue
    "hir.send_cmsg"(%m0) : (!hir.cvalue) -> ()

    %m1 = "hir.meas"(%epr1) : (!hir.qubit) -> !hir.cvalue
    "hir.send_cmsg"(%m1) : (!hir.cvalue) -> ()
    return
}