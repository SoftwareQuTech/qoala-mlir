func @f() -> () {
    %0 = "lir.alloc"() : () -> !lir.qubit
    %1 = "lir.new_cval_c"() : () -> !lir.cvalue_on_clas
    %2 = "lir.cval_c_to_q"(%1) : (!lir.cvalue_on_clas) -> !lir.cvalue_on_quan
    %3 = "lir.gate_x"(%0, %2) : (!lir.qubit, !lir.cvalue_on_quan) -> !lir.qubit
    %4 = "lir.gate_x"(%3, %2) : (!lir.qubit, !lir.cvalue_on_quan) -> !lir.qubit
    %5 = "lir.gate_y"(%4, %2) : (!lir.qubit, !lir.cvalue_on_quan) -> !lir.qubit
    %6 = "lir.entangle"() : () -> !lir.qubit
    %7 = "lir.recv_cmsg"() : () -> !lir.cvalue_on_clas
    "lir.send_cmsg"(%7) : (!lir.cvalue_on_clas) -> ()
    %8 = "lir.meas"(%6) : (!lir.qubit) -> !lir.cvalue_on_quan
    %9 = "lir.cval_q_to_c"(%8) : (!lir.cvalue_on_quan) -> !lir.cvalue_on_clas
    "lir.send_cmsg"(%9) : (!lir.cvalue_on_clas) -> ()
    return
}
