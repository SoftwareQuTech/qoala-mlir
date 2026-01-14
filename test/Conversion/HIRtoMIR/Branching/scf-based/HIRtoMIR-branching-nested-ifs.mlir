// RUN: qoala-opt %s --lower-qoala-hir-to-mir | FileCheck %s

// CHECK: module
module {
  // CHECK: qmem.func @nested_ifs()
  qnet.func @nested_ifs() {
    // CHECK: %[[CST_4:.+]] = arith.constant 4 : i32
    %c4_i32 = arith.constant 4 : i32
    // CHECK: %[[CST_7:.+]] = arith.constant 7 : i32
    %c7_i32 = arith.constant 7 : i32
    // CHECK: %[[CMP_RESULT:.+]] = arith.cmpi sge, %[[CST_4]], %[[CST_7]]
    %0 = arith.cmpi sge, %c4_i32, %c7_i32 : i32
    // CHECK: scf.if %[[CMP_RESULT]] {
    scf.if %0 {
      // CHECK-NEXT: %[[CST_15:.+]] = arith.constant 15 : i32
      %c15_i32 = arith.constant 15 : i32
      // CHECK-NEXT: %[[CST_17:.+]] = arith.constant 17 : i32
      %c17_i32 = arith.constant 17 : i32
      // CHECK-NEXT: %[[CMP_RESULT_B:.+]] = arith.cmpi sle, %[[CST_15]], %[[CST_17]]
      %1 = arith.cmpi sle, %c15_i32, %c17_i32 : i32
      // CHECK: scf.if %[[CMP_RESULT_B]] {
      scf.if %1 {
        // CHECK-NEXT: arith.constant 89 : i32
        %c89_i32 = arith.constant 89 : i32
      // CHECK-NEXT: } else {
      } else {
        // CHECK-NEXT: arith.constant 99 : i32
        %c99_i32 = arith.constant 99 : i32
        // CHECK-NEXT: }
      }
    // CHECK-NEXT: } else {
    } else {
      // CHECK-NEXT: arith.constant 25 : i32
      %c25_i32 = arith.constant 25 : i32
      // CHECK-NEXT: }
    }
    %c30_i32 = arith.constant 30 : i32
    %c10_i32 = arith.constant 10 : i32
    %1 = arith.addi %c30_i32, %c10_i32 : i32
    qnet.return
  }
}
