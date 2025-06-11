// RUN: qoala-opt %s --lower-qoala-mir-to-lir=use-simple-functionize | FileCheck %s

// CHECK: module
module {
  // CHECK: qremote.remote @[[REMOTEBOB:.*]]
  qmem.remote @Bob
  // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

  // CHECK: netqasm.local_routine @[[WRAPPER0:.*]]() -> i32
  // CHECK-NEXT: %[[LOC_QUBIT0:.*]] = netqasm.qalloc  : i32
  // CHECK-NEXT: netqasm.init %[[LOC_QUBIT0]]
  // CHECK-NEXT: netqasm.return %[[LOC_QUBIT0]] : i32

  // CHECK: netqasm.local_routine @[[WRAPPER1:.*]](%[[ARG_QUBITA:.*]]: i32)
  // CHECK-NEXT: netqasm.rot_x %[[ARG_QUBITA]] (0 : ui32, 0 : ui32)
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER2:.*]]() -> i32
  // CHECK-NEXT: %[[LOC_QUBIT1:.*]] = netqasm.qalloc  : i32
  // CHECK-NEXT: netqasm.init %[[LOC_QUBIT1]]
  // CHECK-NEXT: netqasm.return %[[LOC_QUBIT1]] : i32

  // CHECK: netqasm.local_routine @[[WRAPPER3:.*]](%[[ARG_QUBITB:.*]]: i32)
  // CHECK-NEXT: netqasm.rot_y %[[ARG_QUBITB]] (0 : ui32, 0 : ui32)
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER4:.*]](%[[ARG_QUBITC:.*]]: i32, %[[ARG_QUBITD:.*]]: i32)
  // CHECK-NEXT: netqasm.cnot %[[ARG_QUBITC]], %[[ARG_QUBITD]]
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER5:.*]](%[[ARG_QUBITE:.*]]: i32) -> i1
  // CHECK-NEXT: %[[LOC_MEASE:.*]] = netqasm.measure %[[ARG_QUBITE]] : i1
  // CHECK-NEXT: netqasm.return %[[LOC_MEASE]] : i1

  // CHECK: netqasm.local_routine @[[WRAPPER6:.*]](%[[ARG_QUBITF:.*]]: i32) -> i1
  // CHECK-NEXT: %[[LOC_MEASF:.*]] = netqasm.measure %[[ARG_QUBITF]] : i1
  // CHECK-NEXT: netqasm.return %[[LOC_MEASF]] : i1

  // CHECK: netqasm.request_routine @[[WRAPPER7:.*]]() -> i32
  // CHECK-NEXT: %[[LOC_QUBIT2:.*]]  = netqasm.qalloc  : i32
  // CHECK-NEXT: netqasm.eprs %[[LOC_QUBIT2]]  {remote = @Bob}
  // CHECK-NEXT: netqasm.return %[[LOC_QUBIT2]] : i32

  // CHECK: netqasm.local_routine @[[WRAPPER8:.*]](%[[ARG_QUBITG:.*]]: i32)
  // CHECK-NEXT: netqasm.rot_z %[[ARG_QUBITG]] (0 : ui32, 0 : ui32)
  // CHECK-NEXT: netqasm.return

  // CHECK: netqasm.local_routine @[[WRAPPER9:.*]](%[[ARG_QUBITH:.*]]: i32) -> i1
  // CHECK-NEXT: %[[LOC_MEASF:.*]] = netqasm.measure %[[ARG_QUBITH]] : i1
  // CHECK-NEXT: netqasm.return %[[LOC_MEASF]] : i1

  // CHECK: netqasm.request_routine @[[WRAPPER10:.*]]() -> i1
  // CHECK-NEXT: %[[LOC_QUBIT3:.*]] = netqasm.qalloc  : i32
  // CHECK-NEXT: %[[LOC_MEASG:.*]] = netqasm.eprs_measure %[[LOC_QUBIT3]] {remote = @[[REMOTEBOB]]}
  // CHECK-NEXT: netqasm.return %[[LOC_MEASG]] : i1

  // CHECK: qoalahost.main_func @test_entangle_quantum_program()
  qmem.func @test_entangle_quantum_program() {
    // Some programmers like to "declare" all variables at the beginning...
    // CHECK: qoalahost.blk_meta {block_id = "block_0", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %q0 = qmem.qalloc : i32
    %q1 = qmem.qalloc : i32
    %q2 = qmem.qalloc : i32
    %q3 = qmem.qalloc : i32
    %c0 = arith.constant 0.0 : f32

    // Then start working with them.
    // CHECK-NEXT: %[[QUBIT0:.*]] = qoalahost.call @[[WRAPPER0]]() : () -> i32
    qmem.init %q0
    // CHECK: ^[[BLOCK_1:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_1", dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER1]](%[[QUBIT0]]) : (i32) -> ()
    qmem.rot_x %q0, %c0

    // CHECK: ^[[BLOCK_2:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_2", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[QUBIT1:.*]] = qoalahost.call @[[WRAPPER2]]() : () -> i32
    qmem.init %q1
    // CHECK: ^[[BLOCK_3:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_3", dependencies = ["block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER3]](%[[QUBIT1]]) : (i32) -> ()
    qmem.rot_y %q1, %c0

    // CHECK: ^[[BLOCK_4:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_4", dependencies = ["block_1", "block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER4]](%[[QUBIT1]], %[[QUBIT0]]) : (i32, i32) -> ()
    qmem.cnot %q1, %q0

    // CHECK: ^[[BLOCK_5:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_5", dependencies = ["block_4"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[UNUSED_0:.*]] = qoalahost.call @[[WRAPPER5]](%[[QUBIT0]]) : (i32) -> i1
    %unused0 = qmem.measure %q0 : i1

    // CHECK: ^[[BLOCK_6:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_6", dependencies = ["block_4"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[UNUSED_1:.*]] = qoalahost.call @[[WRAPPER6]](%[[QUBIT1]]) : (i32) -> i1
    %unused1 = qmem.measure %q1 : i1

    // Late "init" of an eprs qubit
    // CHECK: ^[[BLOCK_7:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_7", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[QUBIT2:.*]] = qoalahost.call @[[WRAPPER7]]() : () -> i32
    qmem.eprs %q2 {remote = @Bob}

    // CHECK: ^[[BLOCK_8:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_8", dependencies = ["block_7"], predecessors = [], prev_comm = "", prev_ent = ""}
    // Eprs operation is a barrier, so rot_z cannot be grouped together with eprs
    // CHECK-NEXT: qoalahost.call @[[WRAPPER8]](%[[QUBIT2]]) : (i32) -> ()
    qmem.rot_z %q2, %c0

    // CHECK: ^[[BLOCK_9:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_9", dependencies = ["block_8"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[UNUSED_3:.*]] = qoalahost.call @[[WRAPPER9]](%[[QUBIT2]]) : (i32) -> i1
    %unused2 = qmem.measure %q2 : i1

    // CHECK: ^[[BLOCK_10:.*]]:
    // Why does this block depend on "block_7"???
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_10", dependencies = [], predecessors = [], prev_comm = "", prev_ent = "block_7"}
    // CHECK-NEXT: %[[UNUSED_4:.*]] = qoalahost.call @[[WRAPPER10]]() : () -> i1
    %unused3 = qmem.eprs_measure %q3 {remote = @Bob} : i1

    // CHECK: ^[[BLOCK_11:.*]]:
    // CHECK-NEXT: qoalahost.blk_meta {block_id = "block_11", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.return
    qmem.return
  }
}