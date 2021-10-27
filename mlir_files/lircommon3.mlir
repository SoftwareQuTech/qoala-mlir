func @f() -> () {
    %1 = "lir.new_cval_q"() : () -> !lir.cvalue_q
    %q = "lir.alloc"() : () -> !lir.qubit
    %q2 = "lir.rot_x"(%q, %1) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    return
}
