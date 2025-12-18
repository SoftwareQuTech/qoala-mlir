// RUN: qoala-opt %s --lower-qoala-mir-to-lir=max-ops-per-group=1 --qoala-opt-group-ent-reqs | FileCheck %s

// CEHCK: module
module {
    qmem.remote @Bob
    // CHECK: qremote.remote @[[REMOTEBOB:.*]]

    // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

    // CHECK: netqasm.request_routine @[[WRAPPER0:.*]]()
    // CHECK-NEXT: %[[REG0_0:.*]] = netqasm.qalloc : i32
    // CHECK-NEXT: netqasm.eprs %[[REG0_0]] {remote = @[[REMOTEBOB]]}
    // CHECK-NEXT: %[[REG0_1:.*]] = netqasm.qalloc : i32
    // CHECK-NEXT: netqasm.eprs %[[REG0_1]] {remote = @[[REMOTEBOB]]}
    // CHECK-NEXT: netqasm.return

    // CHECK: netqasm.local_routine @[[WRAPPER1:.*]]()
    // CHECK-NEXT: %[[REG0_2:.*]] = netqasm.qalloc : i32
    // CHECK-NEXT: netqasm.init %[[REG0_2]]
    // CHECK-NEXT: netqasm.return

    // CHECK: netqasm.local_routine @[[WRAPPER2:.*]](%[[ARG0_0:.*]]: i32)
    // CHECK-NEXT: netqasm.hadamard %[[ARG0_0]]
    // CHECK-NEXT: netqasm.return

    // CHECK: qoalahost.main_func @test_group_ent()

    // CHECK: qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER0]]() : () -> ()

    // CHECK: qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: %[[REG_MAIN0:.*]] = qoalahost.call @[[WRAPPER1]]() : () -> i32

    // CHECK: qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NEXT: qoalahost.call @[[WRAPPER2]](%[[REG_MAIN0]]) : (i32) -> ()
    qmem.func @test_group_ent() {
        %0 = qmem.qalloc : i32
        qmem.init %0

        %1 = qmem.qalloc : i32
        qmem.eprs %1 {remote = @Bob}

        qmem.hadamard %0

        %2= qmem.qalloc : i32
        qmem.eprs %2 {remote = @Bob}

        qmem.return
    }
}
