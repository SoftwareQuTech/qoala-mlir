func @f() -> () {
    %0 = "lircommon.alloc"() : () -> !lircommon.qubit
    %1 = "lircommon.new_cval_c"() : () -> !lircommon.cvalue_on_clas
    %2 = "lircommon.cval_c_to_q"(%1) : (!lircommon.cvalue_on_clas) -> !lircommon.cvalue_on_quan
    %3 = "lircommon.gate_x"(%0, %2) : (!lircommon.qubit, !lircommon.cvalue_on_quan) -> !lircommon.qubit
    %4 = "lircommon.gate_x"(%3, %2) : (!lircommon.qubit, !lircommon.cvalue_on_quan) -> !lircommon.qubit
    %5 = "lircommon.gate_y"(%4, %2) : (!lircommon.qubit, !lircommon.cvalue_on_quan) -> !lircommon.qubit
    %6 = "lircommon.entangle"() : () -> !lircommon.qubit
    %7 = "lircommon.recv_cmsg"() : () -> !lircommon.cvalue_on_clas
    "lircommon.send_cmsg"(%7) : (!lircommon.cvalue_on_clas) -> ()
    %8 = "lircommon.meas"(%6) : (!lircommon.qubit) -> !lircommon.cvalue_on_quan
    %9 = "lircommon.cval_q_to_c"(%8) : (!lircommon.cvalue_on_quan) -> !lircommon.cvalue_on_clas
    "lircommon.send_cmsg"(%9) : (!lircommon.cvalue_on_clas) -> ()
    return
}
