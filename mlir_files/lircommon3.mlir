func @f() -> () {
    %1 = "lir.new_cval_q"() : () -> !lir.cvalue_on_quan
    %q = "lir.alloc"() : () -> !lir.qubit
    %q2 = "lir.gate_x"(%q, %1) : (!lir.qubit, !lir.cvalue_on_quan) -> !lir.qubit
    return
}
