func @f() -> () {
    %0 = "lir.alloc"() : () -> !lir.qubit
    %1 = "lir.recv_cmsg"() : () -> !lir.cvalue_on_clas
    %2 = "lir.cval_c_to_q"(%1) : (!lir.cvalue_on_clas) -> !lir.cvalue_on_quan
    %3 = "lir.gate_x"(%0, %2) : (!lir.qubit, !lir.cvalue_on_quan) -> !lir.qubit
    %4 = "lir.gate_x"(%3, %2) : (!lir.qubit, !lir.cvalue_on_quan) -> !lir.qubit
    %5 = "lir.meas"(%4) : (!lir.qubit) -> !lir.cvalue_on_quan
    %6 = "lir.cval_q_to_c"(%5) : (!lir.cvalue_on_quan) -> !lir.cvalue_on_clas
    "lir.send_cmsg"(%6) : (!lir.cvalue_on_clas) -> ()
    return
}