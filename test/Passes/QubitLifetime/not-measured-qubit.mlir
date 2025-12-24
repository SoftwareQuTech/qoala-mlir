// RUN: qoala-opt %s --qoalahost-show-analysis-qubit-life | FileCheck %s
// CHECK: [Qubits Lifetimes]:
// CHECK: - 1::2: 1262
// CHECK: - 2::2: 71
// CHECK: - 4::2: 157

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @local_qubit0() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.hadamard %0
    netqasm.return %0 : i32
  }
  netqasm.local_routine @local_qubit1() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.hadamard %0
    netqasm.return %0 : i32
  }
  netqasm.request_routine @entanglement() -> i32 {
    %1 = netqasm.qalloc  : i32
    netqasm.eprs %1  {remote = @Bob}
    netqasm.return %1 : i32
  }
  netqasm.local_routine @cnot0(%0: i32, %1: i32) {
    netqasm.hadamard %0
    netqasm.cnot %0, %1
    netqasm.return
  }
  netqasm.local_routine @cnot1(%0: i32, %1: i32) {
    netqasm.hadamard %0
    netqasm.cnot %0, %1
    netqasm.return
  }
  netqasm.local_routine @cnot2(%0: i32, %1: i32, %2: i32) {
    netqasm.hadamard %0
    netqasm.hadamard %1
    netqasm.cnot %0, %1
    netqasm.hadamard %2
    netqasm.return
  }
  netqasm.local_routine @meas_local0(%0: i32) -> i1 {
    %m = netqasm.measure %0 : i1
    netqasm.return %m : i1
  }
  netqasm.local_routine @meas_local1(%0: i32) -> i1 {
    %m = netqasm.measure %0 : i1
    netqasm.return %m : i1
  }
  netqasm.local_routine @meas_ent(%0: i32) -> i1 {
    %m = netqasm.measure %0 : i1
    netqasm.return %m : i1
  }
  qoalahost.main_func @test_reordering_teleport() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.remote_id_ref  {classical = false, quantum = true, remote = @Bob}
    qoalahost.nop_term
    ^bb1:
        qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        %1 = qoalahost.call @local_qubit0() : () -> i32
    ^bb2:
        qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        %2 = qoalahost.call @local_qubit1() : () -> i32
    ^bb3:
        qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_1", "block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.call @cnot0(%1, %2) : (i32, i32) -> ()
    ^bb4:
        qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        %0 = qoalahost.call @entanglement() : () -> i32
    ^bb5:
        qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = ["block_3", "block_4"], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.call @cnot1(%0, %1) : (i32, i32) -> ()
    ^bb6:
        qoalahost.blk_meta  {block_id = "block_6", deadlines = {}, dependencies = ["block_3", "block_5"], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.call @cnot2(%0, %1, %2) : (i32, i32, i32) -> ()
    ^bb7:
        qoalahost.blk_meta  {block_id = "block_7", deadlines = {}, dependencies = ["block_6"], predecessors = [], prev_comm = "", prev_ent = ""}
        %m0 = qoalahost.call @meas_ent(%0) : (i32) -> i1
    ^bb8:
        qoalahost.blk_meta  {block_id = "block_8", deadlines = {}, dependencies = ["block_6"], predecessors = [], prev_comm = "", prev_ent = ""}
        %m1 = qoalahost.call @meas_local0(%1) : (i32) -> i1
  }
}
