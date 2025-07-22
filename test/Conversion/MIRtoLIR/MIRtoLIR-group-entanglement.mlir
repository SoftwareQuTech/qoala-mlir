// RUN: qoala-opt %s --lower-qoala-mir-to-lir=max-ops-per-group=1 --qoala-opt-group-ent-reqs | FileCheck %s

// CHECK: module
module {
    qmem.remote @Bob
    qmem.remote @Charlie
    // CHECK: qremote.remote @[[REMOTECHARLIE:.*]]
    // CHECK: qremote.remote @[[REMOTEBOB:.*]]

    // CHECK-LABEL: netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)

    // CHECK: netqasm.request_routine @[[WRAPPER0:.*]]() -> (i1, i1)
    // CHECK-NEXT: %[[REG0_0:.*]] = netqasm.qalloc : i32
    // CHECK-NEXT: %[[REG0_1:.*]] = netqasm.eprs_measure %[[REG0_0]] {remote = @[[REMOTECHARLIE]]} : i1
    // CHECK-NEXT: %[[REG0_2:.*]] = netqasm.qalloc : i32
    // CHECK-NEXT: %[[REG0_3:.*]] = netqasm.eprs_measure %[[REG0_2]] {remote = @[[REMOTECHARLIE]]} : i1
    // CHECK-NEXT: netqasm.return %[[REG0_1]], %[[REG0_3]] : i1, i1

    // CHECK: netqasm.request_routine @[[WRAPPER1:.*]]() -> i1
    // CHECK-NEXT: %[[REG0_4:.*]] = netqasm.qalloc : i32
    // CHECK-NEXT: %[[REG0_5:.*]] = netqasm.eprs_measure %[[REG0_4]] {remote = @[[REMOTEBOB]]} : i1
    // CHECK-NEXT: netqasm.return %[[REG0_5]] : i1

    // CHECK: netqasm.request_routine @[[WRAPPER2:.*]]() -> i32
    // CHECK-NEXT: %[[REG0_6:.*]] = netqasm.qalloc : i32
    // CHECK-NEXT: netqasm.eprs %[[REG0_6]] {remote = @[[REMOTEBOB]]}
    // CHECK-NEXT: %[[REG0_7:.*]] = netqasm.qalloc : i32
    // CHECK-NEXT: netqasm.eprs %[[REG0_7]] {remote = @[[REMOTEBOB]]}
    // CHECK-NEXT: netqasm.return %[[REG0_6]] : i32

    // CHECK: netqasm.request_routine @[[WRAPPER3:.*]]()
    // CHECK-NEXT: %[[REG0_8:.*]] = netqasm.qalloc : i32
    // CHECK-NEXT: netqasm.eprs %[[REG0_8]] {remote = @[[REMOTECHARLIE]]}
    // CHECK-NEXT: netqasm.return

    // CHECK: netqasm.local_routine @[[WRAPPER4:.*]](%[[ARG0_0:.*]]: i32)
    // CHECK-NEXT: netqasm.hadamard %[[ARG0_0]]
    // CHECK-NEXT: netqasm.return

    // CHECK: netqasm.local_routine @[[WRAPPER5:.*]](%[[ARG0_1:.*]]: i32)
    // CHECK-NEXT: netqasm.hadamard %[[ARG0_1]]
    // CHECK-NEXT: netqasm.return

    // CHECK: qoalahost.main_func @test_group_ent()
    qmem.func @test_group_ent() {
        // CHECK: qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        // CHECK-NEXT: %[[REG_MAIN0:.*]] = qoalahost.call @[[WRAPPER2]]() : () -> i32
        %0 = qmem.qalloc : i32
        qmem.eprs %0 {remote = @Bob}

        %1 = qmem.qalloc : i32
        qmem.eprs %1 {remote = @Bob}

        // CHECK: qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = "block_0"}
        // CHECK-NEXT: qoalahost.call @[[WRAPPER3]]() : () -> ()
        %2 = qmem.qalloc : i32
        qmem.eprs %2 {remote = @Charlie}

        // CHECK: qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = "block_1"}
        // CHECK-NEXT: qoalahost.call @[[WRAPPER0]]() : () -> (i1, i1)
        %3 = qmem.qalloc : i32
        %4 = qmem.eprs_measure %3 {remote = @Charlie} : i1

        %5 = qmem.qalloc : i32
        %6 = qmem.eprs_measure %5 {remote = @Charlie} : i1

        // CHECK: qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = "block_2"}
        // CHECK-NEXT: %[[REG_MAIN1:.*]] = qoalahost.call @[[WRAPPER1]]() : () -> i1
        %7 = qmem.qalloc : i32
        %8 = qmem.eprs_measure %7 {remote = @Bob} : i1

        // CHECK: qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
        // CHECK-NEXT: qoalahost.call @[[WRAPPER4]](%[[REG_MAIN0]]) : (i32) -> ()
        qmem.hadamard %0

        // CHECK: qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = ["block_4"], predecessors = [], prev_comm = "", prev_ent = ""}
        // CHECK-NEXT: qoalahost.call @[[WRAPPER5]](%[[REG_MAIN0]]) : (i32) -> ()
        qmem.hadamard %0

        // CHECK: qoalahost.blk_meta  {block_id = "block_6", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        // CHECK-NEXT: qoalahost.return
        qmem.return
    }
}
