// RUN: qoala-opt %s --lower-qoala-mir-to-lir=use-simple-functionize | FileCheck %s

// CHECK: module
module {
  // CHECK: qremote.remote @[[REMOTEBOB:.*]]
  qmem.remote @Bob
  // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

  // CHECK: netqasm.local_routine @[[WRAPPER0:.*]]() -> i32
  // CHECK-NEXT: %[[LOC_QUBIT0:.*]] = netqasm.qalloc  : i32
  // CHECK-NEXT: netqasm.init %[[LOC_QUBIT0]]
  // CHECK-NEXT: netqasm.rot_x %[[LOC_QUBIT0]] (0 : ui32, 0 : ui32)
  // CHECK-NEXT: netqasm.return %[[LOC_QUBIT0]] : i32

  // CHECK: netqasm.local_routine @[[WRAPPER1:.*]]() -> i32
  // CHECK-NEXT: %[[LOC_QUBIT1:.*]] = netqasm.qalloc  : i32
  // CHECK-NEXT: netqasm.init %[[LOC_QUBIT1]]
  // CHECK-NEXT: netqasm.rot_y %[[LOC_QUBIT1]] (0 : ui32, 0 : ui32)
  // CHECK-NEXT: netqasm.return %[[LOC_QUBIT1]] : i32

  // CHECK: netqasm.local_routine @[[WRAPPER2:.*]](%[[LOC_QUBITA:.*]]: i32, %[[LOC_QUBITB:.*]]: i32) {
  // CHECK-NEXT: netqasm.cnot %[[LOC_QUBITA]], %%[[LOC_QUBITB]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER3:.*]](%[[LOC_QUBITC:.*]]: i32) -> i1 {
  // CHECK-NEXT: %[[LOC_MEASC:.*]] = netqasm.measure %[[LOC_QUBITC]] : i1
  // CHECK-NEXT: netqasm.return %[[LOC_MEASC]] : i1

  // CHECK: netqasm.local_routine @[[WRAPPER4:.*]](%[[LOC_QUBITD:.*]]: i32) -> i1 {
  // CHECK-NEXT: %[[LOC_MEASD:.*]] = netqasm.measure %[[LOC_QUBITD]] : i1
  // CHECK-NEXT: netqasm.return %[[LOC_MEASD]] : i1

  // CHECK: netqasm.local_routine @[[WRAPPER5:.*]]() -> i32 {
  // CHECK-NEXT: %[[LOC_QUBIT2:.*]]  = netqasm.qalloc  : i32
  // CHECK-NEXT: netqasm.eprs %[[LOC_QUBIT2]]  {remote = @Bob}
  // CHECK-NEXT: netqasm.return %[[LOC_QUBIT2]] : i32

  // CHECK: netqasm.local_routine @[[WRAPPER6:.*]](%[[LOC_QUBITE:.*]]: i32) {
  // CHECK-NEXT: netqasm.rot_z %[[LOC_QUBITE]] (0 : ui32, 0 : ui32)
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER7:.*]](%[[LOC_QUBITF:.*]]: i32) -> i1 {
  // CHECK-NEXT: %[[LOC_MEASF:.*]] = netqasm.measure %[[LOC_QUBITF]] : i1
  // CHECK-NEXT: netqasm.return %[[LOC_MEASC]] : i1

  // CHECK: qoalahost.main_func @test_entangle_quantum_program()
  qmem.func @test_entangle_quantum_program() {
    // Some programmers like to "declare" all variables at the beginning...
    // CHECK: qoalahost.blk_meta {block_id = "block_0", predecessors = []}
    %q0 = qmem.qalloc : i32
    %q1 = qmem.qalloc : i32
    %q2 = qmem.qalloc : i32
    %c0 = arith.constant 0.0 : f32

    // Then start working with them
    // CHECK-NEXT: %[[QUBIT0:.*]] = qoalahost.call @[[WRAPPER0]]() : () -> i32
    qmem.init %q0
    qmem.rot_x %q0, %c0

    // CHECK: ^[[BLOCK_1:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_1", predecessors = []}
    // CHECK-NEXT: %[[QUBIT1:.*]] = qoalahost.call @[[WRAPPER1]]() : () -> i32
    qmem.init %q1
    qmem.rot_y %q1, %c0

    // CHECK: ^[[BLOCK_2:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_2", predecessors = ["block_0", "block_1"]}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER2]](%[[QUBIT0]], %[[QUBIT1]]) : (i32, i32)
    qmem.cnot %q1, %q0

    // CHECK: ^[[BLOCK_3:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_3", predecessors = ["block_0"]}
    // CHECK-NEXT: %[[UNUSED_0:.*]] = qoalahost.call @[[WRAPPER3]](%[[QUBIT0]]) : () -> i1
    %unused0 = qmem.measure %q0 : i1

    // CHECK: ^[[BLOCK_4:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_4", predecessors = ["block_1"]}
    // CHECK-NEXT: %[[UNUSED_1:.*]] = qoalahost.call @[[WRAPPER4]](%[[QUBIT1]]) : () -> i1
    %unused1 = qmem.measure %q1 : i1

    // Late "init" of an eprs qubit
    // CHECK: ^[[BLOCK_5:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_5", predecessors = []}
    // CHECK-NEXT: %[[QUBIT2:.*]] = qoalahost.call @[[WRAPPER5]]() : () -> i32
    qmem.eprs %q2 {remote = @Bob}

    // CHECK: ^[[BLOCK_6:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_6", predecessors = ["block_5"]}
    // Eprs operation is a barrier, so rot_z cannot be grouped together with eprs
    // CHECK-NEXT: qoalahost.call @[[WRAPPER6]](%[[QUBIT2]]) : ()
    qmem.rot_z %q2, %c0

    // CHECK: ^[[BLOCK_7:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_7", predecessors = ["block_5"]}
    // CHECK-NEXT: %[[UNUSED_3:.*]] = qoalahost.call @[[WRAPPER7]](%[[QUBIT2]]) : (i32) -> i1
    %unused2 = qmem.measure %q2 : i1
    qmem.return
  }
}