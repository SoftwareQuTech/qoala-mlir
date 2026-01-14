// RUN: qoala-opt %s --lower-qoala-hir-to-mir | FileCheck %s

// CHECK: module
module {
  // CHECK: qmem.func @simple_if()
  qnet.func @simple_if() {
    // CHECK: %[[CST_4:.+]] = arith.constant 4 : i32
    %c4_i32 = arith.constant 4 : i32
    // CHECK: %[[CST_7:.+]] = arith.constant 7 : i32
    %c7_i32 = arith.constant 7 : i32
    // CHECK:  %[[CMP_RESULT:.+]] = arith.cmpi sgt, %[[CST_4]], %[[CST_7]]
    %0 = arith.cmpi sgt, %c4_i32, %c7_i32 : i32
    // Since we can't capture forward references (block names that are defined later),
    // we need to assert the block namnes textually.
    // CHECK: cf.cond_br %[[CMP_RESULT]], ^bb1, ^bb2
    cf.cond_br %0, ^bb1, ^bb2
  ^bb1:
    %c15_i32 = arith.constant 15 : i32
    // CHECK: cf.br ^bb3
    cf.br ^bb3
  ^bb2:
    %c25_i32 = arith.constant 25 : i32
    // CHECK: cf.br ^bb3
    cf.br ^bb3
  ^bb3:
    %c30_i32 = arith.constant 30 : i32
    %c10_i32 = arith.constant 10 : i32
    %1 = arith.addi %c30_i32, %c10_i32 : i32
    qnet.return
  }
}
