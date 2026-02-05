// RUN: qoala-opt %s --lower-qoala-hir-to-mir | FileCheck %s

// CHECK: module
module {
  // CHECK: qmem.remote @[[BOBREMOTE:.*]]
  qnet.remote @Bob
  // CHECK: qmem.func @simple_if()
  qnet.func @simple_if() {
    %true = arith.constant 1 : i1
    // CHECK: %[[CST_4:.+]] = arith.constant 4 : i32
    %c4_i32 = arith.constant 4 : i32
    // CHECK: %[[RCVINT:.*]] = qmem.recv_int  {remote = @[[BOBREMOTE]]} : i32
    %received_int = qnet.recv_int {remote = @Bob} : i32
    // CHECK:  %[[CMP_RESULT:.+]] = arith.cmpi slt, %[[CST_4]], %[[RCVINT]]
    %0 = arith.cmpi slt, %c4_i32, %received_int : i32
    // CHECK: scf.if %[[CMP_RESULT]] {
    scf.if %0 {
      // CHECK-NEXT: cf.assert %true, "Branch true"
      cf.assert %true, "Branch true"
    // CHECK-NEXT: } else {
    } else {
      // CHECK-NEXT: cf.assert %true, "Branch false"
      cf.assert %true, "Branch false"
    }
    %c30_i32 = arith.constant 30 : i32
    %1 = arith.addi %c30_i32, %received_int : i32
    qnet.return
  }
}
