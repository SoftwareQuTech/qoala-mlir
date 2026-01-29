// RUN: qoala-opt %s --lower-qoala-hir-to-mir | FileCheck %s

// CHECK: module
module {
  // CHECK: qmem.remote @[[BOBREMOTE:.*]]
  qnet.remote @Bob
  // CHECK: qmem.func @nested_ifs()
  qnet.func @nested_ifs() {
    %true = arith.constant 1 : i1
    // CHECK: %[[CST_4:.+]] = arith.constant 4 : i32
    %c4_i32 = arith.constant 4 : i32
    // CHECK: %[[RCVINT_A:.*]] = qmem.recv_int  {remote = @[[BOBREMOTE]]} : i32
    %received_int_a = qnet.recv_int {remote = @Bob} : i32
    // CHECK: %[[RCVINT_B:.*]] = qmem.recv_int  {remote = @[[BOBREMOTE]]} : i32
    %received_int_b = qnet.recv_int {remote = @Bob} : i32
    // CHECK: %[[CMP_RESULT:.+]] = arith.cmpi sge, %[[CST_4]], %[[RCVINT_A]]
    %0 = arith.cmpi sge, %c4_i32, %received_int_a : i32
    // CHECK: scf.if %[[CMP_RESULT]] {
    scf.if %0 {
      // CHECK-NEXT: %[[CST_15:.+]] = arith.constant 15 : i32
      %c15_i32 = arith.constant 15 : i32
      // CHECK-NEXT: %[[CMP_RESULT_B:.+]] = arith.cmpi sle, %[[CST_15]], %[[RCVINT_B]]
      %1 = arith.cmpi sle, %c15_i32, %received_int_b : i32
      // CHECK: scf.if %[[CMP_RESULT_B]] {
      scf.if %1 {
        // CHECK-NEXT: cf.assert %true, "Branch true-true"
        cf.assert %true, "Branch true-true"
      // CHECK-NEXT: } else {
      } else {
        // CHECK-NEXT: cf.assert %true, "Branch true-false"
        cf.assert %true, "Branch true-false"
        // CHECK-NEXT: }
      }
    // CHECK-NEXT: } else {
    } else {
      // CHECK-NEXT: cf.assert %true, "Branch false"
      cf.assert %true, "Branch false"
      // CHECK-NEXT: }
    }
    %c30_i32 = arith.constant 30 : i32
    %1 = arith.addi %c30_i32, %received_int_b : i32
    qnet.return
  }
}
