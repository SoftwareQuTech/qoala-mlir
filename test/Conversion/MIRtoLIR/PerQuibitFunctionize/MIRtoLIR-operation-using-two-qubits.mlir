// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: module
module {
    // CHECK: qremote.remote @[[REMOTEBOB:.*]]
    qmem.remote @Bob
    // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

    // CHECK: netqasm.request_routine @[[WRAPPER0:.*]]() -> i32
    // CHECK-NEXT: %[[QUBIT0:.*]] = netqasm.qalloc : i32
    // CHECK-NEXT: netqasm.eprs %[[QUBIT0]] {remote = @[[REMOTEBOB]]}
    // CHECK-NEXT: netqasm.return %[[QUBIT0]] : i32

    // CHECK: netqasm.local_routine @[[WRAPPER1:.*]]() -> i32
    // CHECK-NEXT: %[[QUBIT1:.*]] = netqasm.qalloc : i32
    // CHECK-NEXT: netqasm.init %[[QUBIT1]]
    // CHECK-NEXT: netqasm.return %[[QUBIT1]] : i32

    // CHECK: netqasm.local_routine @[[WRAPPER2:.*]](%[[QARG0:.*]]: i32, %[[QARG1:.*]]: i32)
    // CHECK-NEXT: netqasm.cnot %[[QARG0]], %[[QARG1]]
    // CHECK-NEXT: netqasm.return

    // CHECK: netqasm.local_routine @[[WRAPPER3:.*]](%[[QARGA:.*]]: i32) -> i1
    // CHECK-NEXT: %[[VAL0:.*]] = netqasm.measure %[[QARGA]] : i1
    // CHECK-NEXT: netqasm.return %[[VAL0]] : i1

    // CHECK: netqasm.local_routine @[[WRAPPER4:.*]](%[[QARGB:.*]]: i32) -> i1
    // CHECK-NEXT: %[[VAL1:.*]] = netqasm.measure %[[QARGB]] : i1
    // CHECK-NEXT: netqasm.return %[[VAL1]] : i1

    // CHECK: qoalahost.main_func @test_functionize_op_using_two_qubits()
    qmem.func @test_functionize_op_using_two_qubits() {
        // CHECK: qoalahost.blk_meta {block_id = "[[BLOCK_1:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        // CHECK-NEXT: %[[MAIN_QUBIT_0:.*]] = qoalahost.call @[[WRAPPER0]]() : () -> i32
        %q0 = qmem.qalloc : i32
        qmem.eprs %q0 {remote = @Bob}

        // CHECK: ^[[BLK_1:.*]]:
        // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_2:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        // CHECK-NEXT: %[[MAIN_QUBIT_1:.*]] = qoalahost.call @[[WRAPPER1]]() : () -> i32
        %q1 = qmem.qalloc : i32
        qmem.init %q1

        // CHECK: ^[[BLK_2:.*]]:
        // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_3:.*]]", deadlines = {}, dependencies = ["[[BLOCK_1]]", "[[BLOCK_2]]"], predecessors = [], prev_comm = "", prev_ent = ""}
        // CHECK-NEXT: qoalahost.call @[[WRAPPER2]](%[[MAIN_QUBIT_0]], %[[MAIN_QUBIT_1]]) : (i32, i32) -> ()
        qmem.cnot %q0, %q1

        // CHECK: ^[[BLK_3:.*]]:
        // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_4:.*]]", deadlines = {}, dependencies = ["[[BLOCK_3]]"], predecessors = [], prev_comm = "", prev_ent = ""}
        // CHECK-NEXT: %[[UNUSED_A:.*]] = qoalahost.call @[[WRAPPER3]](%[[MAIN_QUBIT_0]]) : (i32) -> i1
        %m0 = qmem.measure %q0 : i1

        // CHECK: ^[[BLK_4:.*]]:
        // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_5:.*]]", deadlines = {}, dependencies = ["[[BLOCK_3]]"], predecessors = [], prev_comm = "", prev_ent = ""}
        // CHECK-NEXT: %[[UNUSED_B:.*]] = qoalahost.call @[[WRAPPER4]](%[[MAIN_QUBIT_1]]) : (i32) -> i1
        %m1 = qmem.measure %q1 : i1

        // CHECK: ^[[BLK_5:.*]]:
        // CHECK-NEXT: qoalahost.blk_meta {block_id = "[[BLOCK_6:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        // CHECK-NEXT: qoalahost.return
        qmem.return
    }
}