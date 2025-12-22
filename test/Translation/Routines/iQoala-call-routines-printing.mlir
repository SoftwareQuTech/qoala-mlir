// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @local_qubit() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.request_routine @entanglement() -> i32 {
    %1 = netqasm.qalloc  : i32
    netqasm.eprs %1  {remote = @Bob}
    netqasm.return %1 : i32
  }
  netqasm.local_routine @bsm_cnot(%0: i32, %1: i32) {
    netqasm.cnot %0, %1
    netqasm.return
  }
  netqasm.local_routine @bsm_h(%0: i32) {
    netqasm.hadamard %0
    netqasm.return
  }
  netqasm.local_routine @bsm_meas_local(%0: i32) -> i1 {
    %m0 = netqasm.measure %0 : i1
    netqasm.return %m0 : i1
  }
  netqasm.local_routine @bsm_meas_ent(%1: i32) -> i1 {
    %m1 = netqasm.measure %1 : i1
    netqasm.return %m1 : i1
  }
  qoalahost.main_func @test_reordering_teleport() {
    // CHECK: ^b[[BLK_0:.*]]
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    // CHECK-NOT:%{{[0-9]+}} =
    // CHECK:run_request() : entanglement
    %1 = qoalahost.call @entanglement() : () -> i32
    ^bb1:
        // CHECK: ^b[[BLK_1:.*]]
        qoalahost.blk_meta  {block_id = "block_0", deadlines = {block_1 = 396 : i64}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        // CHECK-NOT:%{{[0-9]+}} =
        // CHECK:run_subroutine() : local_qubit
        %0 = qoalahost.call @local_qubit() : () -> i32
    ^bb2:
        qoalahost.blk_meta  {block_id = "block_2", deadlines = {block_1 = 410 : i64}, dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.call @bsm_h(%0) : (i32) -> ()
    ^bb3:
        qoalahost.blk_meta  {block_id = "block_3", deadlines = {block_1 = 422 : i64}, dependencies = ["block_1", "block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.call @bsm_cnot(%0, %1) : (i32, i32) -> ()
    ^bb4:
        qoalahost.blk_meta  {block_id = "block_4", deadlines = {block_1 = 474 : i64}, dependencies = ["block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
        // CHECK: tuple<%[[RET_A:.*]]> = run_subroutine() : bsm_meas_local
        %m0 = qoalahost.call @bsm_meas_local(%0) : (i32) -> i1
    ^bb5:
        qoalahost.blk_meta  {block_id = "block_5", deadlines = {block_1 = 487 : i64}, dependencies = ["block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
        // CHECK: tuple<%[[RET_B:.*]]> = run_subroutine() : bsm_meas_ent
        %m1 = qoalahost.call @bsm_meas_ent(%1) : (i32) -> i1
  }
}
