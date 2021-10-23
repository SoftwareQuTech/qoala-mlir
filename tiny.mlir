func @f() -> !hir.cmsg {
    %0 = "hir.alloc"() : () -> !hir.qubit
    %1 = "hir.gate_x"(%0) : (!hir.qubit) -> !hir.qubit
    %15 = "hir.gate_x"(%1) : (!hir.qubit) -> !hir.qubit
    %2 = "hir.gate_y"(%15) : (!hir.qubit) -> !hir.qubit
    %3 = "hir.entangle"() : () -> !hir.qubit
    %4 = "hir.recv_cmsg"() : () -> !hir.cmsg
    "hir.send_cmsg"(%4) : (!hir.cmsg) -> ()
    %5 = "lir.alloc"() : () -> !lir.qubit
    return %4 : !hir.cmsg
}