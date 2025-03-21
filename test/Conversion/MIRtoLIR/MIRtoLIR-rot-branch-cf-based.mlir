// UNSUPPORTED: true
// Grouping arith+cf.cond_branch instructions inside local routines is not supported yet.
// RUN: qoala-opt %s --lower-qoala-mir-to-lir=use-sccp | FileCheck %s

// CHECK: module
module {
  // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

  // CHECK-LABEL: netqasm.local_routine @[[WRAPPER0:.*]]([[ARG0_0:.*]]: i32) -> i1
  // CHECK: %[[C1:.*]] = arith.constant 1 : i32
  // CHECK: %[[REG0_0:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.init %[[REG0_0]]
  // CHECK-NEXT: %[[BR_RES:.*]] = arith.cmpi eq, %[[ARG0_0]], %[[C1]] : i32
  // CHECK-NEXT: cf.cond_branch %[[BR_RES]], ^[[BB1.:*]], ^[[BB2:*]]
  // CHECK-NEXT: ^[[BB1]]:
  // CHECK-NEXT: netqasm.rot_x %[[REG0_0]] (3 : ui32, 2 : ui32)
  // CHECK-NEXT: cf.br ^[[BB3:.*]]
  // CHECK-NEXT: ^[[BB2]]:
  // CHECK-NEXT: netqasm.rot_y %[[REG0_0]] (1 : ui32, 2 : ui32)
  // CHECK-NEXT: cf.br ^[[BB3:.*]]
  // CHECK-NEXT: ^[[BB3]]:
  // CHECK-NEXT: %[[REG0_1:.*]] = netqasm.measure %[[REG0_0]] : i1
  // CHECK-NEXT: netqasm.return %[[REG0_1]] : i1

  // CHECK: qoalahost.main_func @test_remote_quantum_program(%[[ARG0:.*]]: i32) -> i1
  qmem.func @test_remote_quantum_program(%arg0: i32) -> i1 {
    // The next three constants should flow inside ^bb1 and ^bb2
    // Base constant to compare the argument.
    %c1 = arith.constant 1 : i32
    %cst_0 = arith.constant 2.356194 : f32
    %cst_1 = arith.constant 0.785398 : f32
    // CHECK qoalahost.nop_term

    // CHECK: ^[[BLOCK_1:.*]]:
    // CHECK: %[[CALL_RES:.*]] = qoalahost.call @[[WRAPPER0]](%[[ARG0]]) : (i32) -> i1
    // All the following instructions should be moved inside the netqasm local routine
    // *including* the branching operations, and the branching blocks, to preserve the
    // semantics of the program.
    %0 = qmem.qalloc : i32
    qmem.init %0

    %branch = arith.cmpi eq, %arg0, %c1 : i32
    cf.cond_br %branch, ^bb1(%cst_0: f32), ^bb2(%cst_1: f32)

  ^bb1(%a1: f32):
    qmem.rot_x %0, %a1
    cf.br ^bb3

  ^bb2(%a2: f32):
    qmem.rot_y %0, %a2
    cf.br ^bb3

  ^bb3:
    %2 = qmem.measure %0 : i1

    // CHECK: ^[[BLOCK_2:.*]]:
    // CHECK-NEXT: qoalahost.return %[[CALL_RES]]
    qmem.return %2 : i1
  }
}