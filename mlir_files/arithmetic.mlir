func @f() -> () {
    %c0 = "lircommon.new_cval_q"() : () -> !lircommon.cvalue_on_quan
    %c1 = "lircommon.new_cval_q"() : () -> !lircommon.cvalue_on_quan
    // %c2 = "lircommon.add_cvalue_on_q"(%c0, %c1) : (!lircommon.cvalue_on_quan, !lircommon.cvalue_on_quan) -> !lircommon.cvalue_on_quan

    %q0 = "lircommon.alloc"() : () -> !lircommon.qubit
    %q1 = "lircommon.gate_x"(%q0, %c0) : (!lircommon.qubit, !lircommon.cvalue_on_quan) -> !lircommon.qubit
    %q2 = "lircommon.gate_x"(%q1, %c1) : (!lircommon.qubit, !lircommon.cvalue_on_quan) -> !lircommon.qubit

    return
}