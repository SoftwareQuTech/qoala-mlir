func @f() -> () {
    %c0 = "lir.new_cval_q"() : () -> !lir.cvalue_on_quan
    %c1 = "lir.new_cval_q"() : () -> !lir.cvalue_on_quan
    // %c2 = "lir.add_cvalue_on_q"(%c0, %c1) : (!lir.cvalue_on_quan, !lir.cvalue_on_quan) -> !lir.cvalue_on_quan

    %q0 = "lir.alloc"() : () -> !lir.qubit
    %q1 = "lir.gate_x"(%q0, %c0) : (!lir.qubit, !lir.cvalue_on_quan) -> !lir.qubit
    %q2 = "lir.gate_x"(%q1, %c1) : (!lir.qubit, !lir.cvalue_on_quan) -> !lir.qubit

    return
}