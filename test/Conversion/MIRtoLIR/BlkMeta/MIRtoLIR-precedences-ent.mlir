// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

module {
    qmem.remote @Bob
    qmem.func @test_add_eprs_block_deps() {
        // CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_0:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        %q0 = qmem.qalloc : i32
        qmem.eprs %q0 {remote = @Bob}

        // CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_1:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = "[[BLOCK_0]]"}
        %q1 = qmem.qalloc : i32
        qmem.eprs %q1 {remote = @Bob}

        // CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_2:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = "[[BLOCK_1]]"}
        %q2 = qmem.qalloc : i32
        qmem.eprs %q2 {remote = @Bob}

        // CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_3:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        qmem.return
    }
}
