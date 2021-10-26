func @f() -> () {
    %1 = "lircommon.new_cval_q"() : () -> !lircommon.cvalue_on_quan
    %q = "lircommon.alloc"() : () -> !lircommon.qubit
    %q2 = "lircommon.gate_x"(%q, %1) : (!lircommon.qubit, !lircommon.cvalue_on_quan) -> !lircommon.qubit
    return
}
