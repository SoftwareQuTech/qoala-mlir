func @f() -> () {
    %q0 = "lir.alloc"() : () -> !lir.qubit

    %c0 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %c1 = "lir.new_cval_c"() : () -> !lir.cvalue_c

    %q1 = "lir.alloc"() : () -> !lir.qubit

    "lir.send_cmsg"(%c0) : (!lir.cvalue_c) -> ()
    "lir.send_cmsg"(%c1) : (!lir.cvalue_c) -> ()

    return
}