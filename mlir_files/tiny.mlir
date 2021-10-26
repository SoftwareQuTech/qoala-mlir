func @f() -> () {
    %q0 = "hir.alloc"() : () -> !hir.qubit
    %c = "hir.new_cval"() : () -> !hir.cvalue
    %q1 = "hir.gate_x"(%q0, %c) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %q2 = "hir.gate_x"(%q1, %c) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %q3 = "hir.gate_y"(%q2, %c) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
    %3 = "hir.entangle"() : () -> !hir.qubit
    %4 = "hir.recv_cmsg"() : () -> !hir.cvalue
    "hir.send_cmsg"(%4) : (!hir.cvalue) -> ()
    %5 = "hir.alloc"() : () -> !hir.qubit
    return
}