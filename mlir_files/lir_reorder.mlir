func @f() -> () {
    %0 = "lircommon.alloc"() : () -> !lircommon.qubit
    %1 = "lircommon.recv_cmsg"() : () -> !lircommon.cvalue_on_clas
    %2 = "lircommon.cval_c_to_q"(%1) : (!lircommon.cvalue_on_clas) -> !lircommon.cvalue_on_quan
    %3 = "lircommon.gate_x"(%0, %2) : (!lircommon.qubit, !lircommon.cvalue_on_quan) -> !lircommon.qubit
    %4 = "lircommon.gate_x"(%3, %2) : (!lircommon.qubit, !lircommon.cvalue_on_quan) -> !lircommon.qubit
    %5 = "lircommon.meas"(%4) : (!lircommon.qubit) -> !lircommon.cvalue_on_quan
    %6 = "lircommon.cval_q_to_c"(%5) : (!lircommon.cvalue_on_quan) -> !lircommon.cvalue_on_clas
    "lircommon.send_cmsg"(%6) : (!lircommon.cvalue_on_clas) -> ()
    return
}