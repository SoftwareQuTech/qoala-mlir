func @f() -> () {
    %1 = "lircommon.new_cval_c"() : () -> !lircommon.cvalue_on_clas
    %q = "lircommon.alloc"() : () -> !lircommon.qubit
    "lircommon.send_cmsg"(%1) : (!lircommon.cvalue_on_clas) -> ()
    return
}
