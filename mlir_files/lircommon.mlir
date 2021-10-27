func @f() -> () {
    %0 = "lir.alloc"() : () -> !lir.qubit
    %1 = "lir.new_cval_c"() : () -> !lir.cvalue_c
    %2 = "lir.cval_c_to_q"(%1) : (!lir.cvalue_c) -> !lir.cvalue_q
    %3 = "lir.rot_x"(%0, %2) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %4 = "lir.rot_x"(%3, %2) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %5 = "lir.rot_y"(%4, %2) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %6 = "lir.entangle"() : () -> !lir.qubit
    %7 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    "lir.send_cmsg"(%7) : (!lir.cvalue_c) -> ()
    %8 = "lir.meas"(%6) : (!lir.qubit) -> !lir.cvalue_q
    %9 = "lir.cval_q_to_c"(%8) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%9) : (!lir.cvalue_c) -> ()
    return
}
