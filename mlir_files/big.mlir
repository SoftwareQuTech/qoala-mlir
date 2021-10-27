func @f() -> () {
    %q0 = "hir.entangle"() : () -> !hir.qubit
    %q1 = "hir.entangle"() : () -> !hir.qubit
    %q2 = "hir.entangle"() : () -> !hir.qubit
    %q0a, %q1a = "hir.cphase"(%q0, %q1) : (!hir.qubit, !hir.qubit) -> (!hir.qubit, !hir.qubit)
    %q0b, %q2a = "hir.cphase"(%q0a, %q2) : (!hir.qubit, !hir.qubit) -> (!hir.qubit, !hir.qubit)

    %q0c = "hir.gate_h"(%q0b) : (!hir.qubit) -> !hir.qubit
    %q0d = "hir.gate_h"(%q0c) : (!hir.qubit) -> !hir.qubit
    %q0e = "hir.gate_h"(%q0d) : (!hir.qubit) -> !hir.qubit
    %q0f = "hir.gate_h"(%q0e) : (!hir.qubit) -> !hir.qubit

    %q3 = "hir.alloc"() : () -> !hir.qubit
    %q3a = "hir.gate_h"(%q3) : (!hir.qubit) -> !hir.qubit
    %q3b = "hir.gate_h"(%q3a) : (!hir.qubit) -> !hir.qubit

    %d0 = "hir.recv_cmsg"() : () -> !hir.cvalue
    %c0 = "hir.new_cval"() : () -> !hir.cvalue

    %q1b = "hir.rot_x"(%q1a, %c0) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %q1c = "hir.rot_x"(%q1b, %c0) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %q1d = "hir.rot_x"(%q1c, %c0) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %q1e = "hir.rot_x"(%q1d, %c0) : (!hir.qubit, !hir.cvalue) -> !hir.qubit

    %d1 = "hir.recv_cmsg"() : () -> !hir.cvalue
    %q1f = "hir.rot_z"(%q1e, %d1) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    // %m0 = "hir.meas"(%q0a) : (!hir.qubit) -> !hir.cvalue
    // "hir.send_cmsg"(%m0) : (!hir.cvalue) -> ()
    // %d1 = "hir.recv_cmsg"() : () -> !hir.cvalue
    // %q1b = "hir.rot_z"(%q1a, %d1) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    // %m1 = "hir.meas"(%q1b) : (!hir.qubit) -> !hir.cvalue
    // "hir.send_cmsg"(%m1) : (!hir.cvalue) -> ()
    return
}