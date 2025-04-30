// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

module {
    qmem.remote @Bob
    qmem.func @test_add_eprs_block_deps() {
// CHECK: qoalahost.blk_meta  {block_id = "block_0", predecessors = []}
        %q0 = qmem.qalloc : i32
        qmem.eprs %q0 {remote = @Bob}

// CHECK: qoalahost.blk_meta  {block_id = "block_1", predecessors = ["block_0"]}
        %q1 = qmem.qalloc : i32
        qmem.eprs %q1 {remote = @Bob}

// CHECK: qoalahost.blk_meta  {block_id = "block_2", predecessors = []}
        qmem.return
    }
}
