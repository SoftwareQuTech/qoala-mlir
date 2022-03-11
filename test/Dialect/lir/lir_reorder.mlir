// RUN: hir-opt --lir-apply-reordering-down %s | FileCheck %s

func @f() -> () {
    %0 = "lir.alloc"() : () -> !lir.qubit
    %1 = "lir.recv_cmsg"() : () -> !lir.cvalue_c
    %2 = "lir.cval_c_to_q"(%1) : (!lir.cvalue_c) -> !lir.cvalue_q
    %3 = "lir.rot_x"(%0, %2) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %4 = "lir.rot_x"(%3, %2) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %5 = "lir.meas"(%4) : (!lir.qubit) -> !lir.cvalue_q
    %6 = "lir.cval_q_to_c"(%5) : (!lir.cvalue_q) -> !lir.cvalue_c
    "lir.send_cmsg"(%6) : (!lir.cvalue_c) -> ()
    return
}

// CHECK:       "lir.recv_cmsg"
// CHECK-NEXT:  "lir.cval_c_to_q"
// CHECK-NEXT:  "lir.alloc"
// CHECK-NEXT:  "lir.rot_x"
// CHECK-NEXT:  "lir.rot_x"
// CHECK-NEXT:  "lir.meas"
// CHECK-NEXT:  "lir.cval_q_to_c"
// CHECK-NEXT:  "lir.send_cmsg"