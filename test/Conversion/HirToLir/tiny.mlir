// RUN: hir-opt --convert-hir-to-lir %s | FileCheck %s

%q0 = "hir.alloc"() : () -> !hir.qubit
%c0 = "hir.recv_cmsg"() : () -> !hir.cvalue
%q1 = "hir.gate_x"(%q0, %c0) : (!hir.qubit, !hir.cvalue) -> !hir.qubit
%q2 = "hir.gate_x"(%q1, %c0) : (!hir.qubit, !hir.cvalue) -> !hir.qubit

// CHECK:       %0 = "lir.alloc"() : () -> !lir.qubit
// CHECK-NEXT:  %1 = "lir.recv_cmsg"() : () -> !lir.cvalue_on_clas
// CHECK-NEXT:  %2 = "lir.cval_c_to_q"(%1) : (!lir.cvalue_on_clas) -> !lir.cvalue_on_quan
// CHECK-NEXT:  %3 = "lir.gate_x"(%0, %2) 
// CHECK-NEXT:  %4 = "lir.gate_x"(%3, %2) 
