// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

module {
    qmem.remote @Bob
    qmem.func @test_add_eprs_block_deps() {
        // CHECK: qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        %q0 = qmem.qalloc : i32
        qmem.eprs %q0 {remote = @Bob}

        // CHECK: qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = "block_1"}
        %q1 = qmem.qalloc : i32
        qmem.eprs %q1 {remote = @Bob}

        // CHECK: qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = "block_2"}
        %q2 = qmem.qalloc : i32
        qmem.eprs %q2 {remote = @Bob}

        // CHECK: qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        qmem.return
    }
}
