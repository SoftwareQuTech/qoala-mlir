// RUN: qoala-opt %s --lower-qoala-mir-to-lir=max-ops-per-group=1 | FileCheck %s

// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_0:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_1:.*]]", deadlines = {}, dependencies = ["[[BLOCK_0]]"], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_2:.*]]", deadlines = {}, dependencies = ["[[BLOCK_1]]"], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_3:.*]]", deadlines = {}, dependencies = ["[[BLOCK_2]]"], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_4:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
module {
  qmem.func @test_local_quantum_program() {
    %0 = qmem.qalloc : i32
    qmem.init %0

    %cst_0 = arith.constant 2.356194 : f32
    %cst_1 = arith.constant 0.785398 : f32

    qmem.rot_x %0, %cst_0

    qmem.rot_y %0, %cst_1
    %2 = qmem.measure %0 : i1

    qmem.return
  }
}
