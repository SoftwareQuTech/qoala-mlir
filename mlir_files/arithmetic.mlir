func @f() -> () {
    %c0 = "lir.new_cval_q"() : () -> !lir.cvalue_q
    %c1 = "lir.new_cval_q"() : () -> !lir.cvalue_q
    // %c2 = "lir.add_cval_q"(%c0, %c1) : (!lir.cvalue_q, !lir.cvalue_q) -> !lir.cvalue_q

    %q0 = "lir.alloc"() : () -> !lir.qubit
    %q1 = "lir.rot_x"(%q0, %c0) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %q2 = "lir.rot_x"(%q1, %c1) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit

    return
}