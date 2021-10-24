%q0 = "hir.alloc"() : () -> !hir.qubit
%c0 = "hir.recv_cmsg"() : () -> !hir.cvalue
%q1 = "hir.gate_x"(%q0, %c0) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
%q2 = "hir.gate_x"(%q1, %c0) : (!hir.qubit, !hir.cvalue) -> !hir.qubit


// %q0 = "lircommon.alloc"() : () -> !lircommon.qubit
// %c0 = "lircommon.recv_cmsg"() : () -> !lircommon.cvalue_on_clas
// %c1 = "lircommon.cval_c_to_q"(%c0) : (!lircommon.cvalue_on_clas) -> !lircommon.cvalue_on_quan
// %q1 = "lircommon.gate_x"(%q0, %c1) 
//     : (!lircommon.qubit, !lircommon.cvalue_on_quan) -> !lircommon.qubit
