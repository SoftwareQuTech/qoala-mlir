// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_0:.*]]", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_1:.*]]", deadlines = {}, dependencies = ["[[BLOCK_0]]"], predecessors = ["[[BLOCK_0]]"], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_2:.*]]", deadlines = {}, dependencies = ["[[BLOCK_0]]"], predecessors = ["[[BLOCK_0]]"], prev_comm = "", prev_ent = ""}
// CHECK: qoalahost.blk_meta  {block_id = "[[BLOCK_3:.*]]", deadlines = {}, dependencies = [], predecessors = ["[[BLOCK_1]]", "[[BLOCK_2]]"], prev_comm = "", prev_ent = ""}
module {
  qmem.func @main(%arg0: i32) -> i32 {
    %c10 = arith.constant 10 : i32
    %cmp = arith.cmpi slt, %arg0, %c10 : i32
    cf.cond_br %cmp, ^then_block, ^else_block

  ^then_block:
    %then_val = arith.constant 1 : i32
    cf.br ^end_block(%then_val : i32)

  ^else_block:
    %else_val = arith.constant 0 : i32
    cf.br ^end_block(%else_val : i32)

  ^end_block(%res: i32):
    qmem.return %res : i32
  }
}
