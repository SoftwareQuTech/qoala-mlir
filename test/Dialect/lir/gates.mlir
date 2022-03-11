// RUN: hir-opt --lir-optimize-gates %s | FileCheck %s

func @f() -> () {
    %q0a = "lir.alloc"() : () -> !lir.qubit
// CHECK:       lir.alloc

    %q0b = "lir.gate_h"(%q0a) : (!lir.qubit) -> !lir.qubit
    %q0c = "lir.gate_h"(%q0b) : (!lir.qubit) -> !lir.qubit

    %c1a = "lir.new_cval_q"() : () -> !lir.cvalue_q
    %c1b = "lir.new_cval_q"() : () -> !lir.cvalue_q
    %q0d = "lir.rot_x"(%q0c, %c1a) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
    %q0e = "lir.rot_x"(%q0d, %c1b) : (!lir.qubit, !lir.cvalue_q) -> !lir.qubit
// CHECK-NEXT:  lir.new_cval_q
// CHECK-NEXT:  lir.new_cval_q
// CHECK-NEXT:  lir.add_cval_q
// CHECK-NEXT:  lir.rot_x

    return
// CHECK-NEXT:  return
}
