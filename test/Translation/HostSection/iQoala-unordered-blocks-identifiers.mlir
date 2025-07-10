// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s

// CHECK: ^b0 { type = QC; predecessors = []; dependencies = []; prev_comm = ; prev_ent = }:
// CHECK: ^b1 { type = QL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = }:
// CHECK: ^b2 { type = QL; predecessors = []; dependencies = [b1, b0]; prev_comm = ; prev_ent = }:

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @local_qubit() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.request_routine @entanglement() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0 {remote = @Bob}
    netqasm.return %0 : i32
  }
  netqasm.local_routine @cnot(%arg0: i32, %arg1: i32) {
    netqasm.cnot %arg0, %arg1
    netqasm.return
  }
  qoalahost.main_func @test_reordering_teleport() {
    qoalahost.blk_meta  {block_id = "block_1", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @entanglement() : () -> i32
  ^bb1:
    qoalahost.blk_meta  {block_id = "block_0", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = qoalahost.call @local_qubit() : () -> i32
  ^bb3:
    qoalahost.blk_meta  {block_id = "block_3", dependencies = ["block_0", "block_1"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @cnot(%1, %0) : (i32, i32) -> ()
  ^bb4:
    qoalahost.blk_meta  {block_id = "block_3", dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.return
  }
}
