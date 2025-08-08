// RUN: qoala-opt %s --lower-qoala-mir-to-lir=use-simple-functionize | FileCheck %s

// CHECK: module
module {
  // CHECK: qremote.remote @[[REMOTEBOB:.*]]
  qmem.remote @Bob
  // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

  // Despite we are using simple functionize, qalloc and eprs *must* be packed together,
  // since both operations define a request routine. We test that here
  // CHECK: netqasm.request_routine @[[WRAPPER0:.*]]() -> i32
  // CHECK-NEXT: %[[REG0_0:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.eprs %[[REG0_0]] {remote = @[[REMOTEBOB]]}
  // CHECK-NEXT: netqasm.return %[[REG0_0]] : i32

  // CHECK: netqasm.request_routine @[[WRAPPER1:.*]]() -> i32
  // CHECK-NEXT: %[[REG0_2:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.eprs %[[REG0_2]] {remote = @[[REMOTEBOB]]}
  // CHECK-NEXT: netqasm.return %[[REG0_2]] : i32

  // CHECK: netqasm.request_routine @[[WRAPPER2:.*]]() -> i32
  // CHECK-NEXT: %[[REG0_4:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: netqasm.eprs %[[REG0_4]] {remote = @[[REMOTEBOB]]}
  // CHECK-NEXT: netqasm.return %[[REG0_4]] : i32

  // CHECK: netqasm.local_routine @[[WRAPPER3:.*]](%[[ARG0_6:.*]]: i32)
  // CHECK-NEXT: netqasm.rot_x %[[ARG0_6]] (0 : ui32, 0 : ui32)
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER4:.*]](%[[ARG0_7:.*]]: i32)
  // CHECK-NEXT: netqasm.rot_y %[[ARG0_7]] (0 : ui32, 0 : ui32)
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER5:.*]](%[[ARG0_7:.*]]: i32, %[[ARG1_8:.*]]: i32)
  // CHECK-NEXT: netqasm.cnot %[[ARG0_7]], %[[ARG1_8]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER6:.*]](%[[ARG0_9:.*]]: i32) -> i1
  // CHECK-NEXT: %[[REG0_8:.*]] = netqasm.measure %[[ARG0_9]] : i1
  // CHECK-NEXT: netqasm.return %[[REG0_8]] : i1

  // The same argument applies to the qalloc+eprs operations -> they need to be together
  // CHECK: netqasm.request_routine @[[WRAPPER7:.*]]() -> i1
  // CHECK-NEXT: %[[REG0_9:.*]] = netqasm.qalloc : i32
  // CHECK-NEXT: %[[REG0_10:.*]] = netqasm.eprs_measure %[[REG0_9]] {remote = @[[REMOTEBOB]]} : i1
  // CHECK-NEXT: netqasm.return %[[REG0_10]] : i1

  // CHECK: qoalahost.main_func @test_entangle_quantum_program()
  qmem.func @test_entangle_quantum_program() {
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[REG_MAIN0:.*]] = qoalahost.call @[[WRAPPER0]]() : () -> i32
    %0 = qmem.qalloc : i32
    qmem.eprs %0 {remote = @Bob}

    // CHECK: ^[[BLOCK_1:.*]]:
    // Why does this block depend on "block_0"?
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = "block_0"}
    // CHECK-NEXT: %[[REG_MAIN1:.*]] = qoalahost.call @[[WRAPPER1]]() : () -> i32
    %1 = qmem.qalloc : i32
    qmem.eprs %1 {remote = @Bob}

    // CHECK: ^[[BLOCK_2:.*]]:
    // Why does this block depend on "block_1"?
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = "block_1"}
    // CHECK-NEXT: %[[REG_MAIN2:.*]] = qoalahost.call @[[WRAPPER2]]() : () -> i32
    %2 = qmem.qalloc : i32
    qmem.eprs %2 {remote = @Bob}

    %c0 = arith.constant 0.0 : f32

    // CHECK: ^[[BLOCK_3:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_3", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER3]](%[[REG_MAIN0]]) : (i32) -> ()
    qmem.rot_x %0, %c0

    %c1 = arith.constant 0.0 : f32

    // CHECK: ^[[BLOCK_4:.*]]:
    // Why does this block depend on "block_2"?
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_4", deadlines = {}, dependencies = ["block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER4]](%[[REG_MAIN2]]) : (i32) -> ()
    qmem.rot_y %2, %c1

    // CHECK: ^[[BLOCK_5:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_5", deadlines = {}, dependencies = ["block_1", "block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER5]](%[[REG_MAIN1]], %[[REG_MAIN0]]) : (i32, i32) -> ()
    qmem.cnot %1, %0

    // CHECK: ^[[BLOCK_6:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_6", deadlines = {}, dependencies = ["block_4"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK: %[[REG_MAIN7:.*]] = qoalahost.call @[[WRAPPER6]](%[[REG_MAIN2]]) : (i32) -> i1
    %3 = qmem.measure %2 : i1

    // CHECK: ^[[BLOCK_7:.*]]:
    // Why does this block depend on "block_2"?
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_7", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = "block_2"}
    // CHECK: %[[REG_MAIN8:.*]] = qoalahost.call @[[WRAPPER7]]() : () -> i1
    %4 = qmem.qalloc : i32
    %5 = qmem.eprs_measure %4 {remote = @Bob} : i1

    // CHECK: ^[[BLOCK_8:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_8", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK: qoalahost.return
    qmem.return
  }
}