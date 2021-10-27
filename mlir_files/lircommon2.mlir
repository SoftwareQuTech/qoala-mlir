func @f() -> () {
    %1 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %q = "lir.alloc"() : () -> !lir.qubit
    "lir.send_cmsg"(%1) : (!lir.cvalue_c) -> ()
    return
}
