// RUN: qoala-translate %s --mlir-to-iqoala

// Note: According to the lifetime of a qubit, in LIR a "qubit value" can be returned either when:
// (a) when using netqasm.eprs in a request routine.
// (b) when using netqasm.qalloc + netqasm.qinit in a loca routine.
// There are two cases in LIR that *cannot* return qubit values to the qoalahost section:
// 1. when a local routine uses netqasm.qalloc + netqasm.qinit and then netqasm.measure *within the
//    same local routine*, and
// 2. when a request routine uses netqasm.eprs_measure.
// In these two cases, the lifetime of a qubit is "local" to the routine (either local or request)
// and that (qubit) value is *not* returned back to the qoalahost section.
// These two special cases are tested here

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @local_qubit_alloc_and_measure() -> i1 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    %1 = netqasm.measure %0 : i1
    netqasm.return %1 : i1
  }
  netqasm.request_routine @entangle_measure() -> i1 {
    %0 = netqasm.qalloc  : i32
    %2 = netqasm.eprs_measure %0  {remote = @Bob} : i1
    netqasm.return %2 : i1
  }
  qoalahost.main_func @test_reordering_teleport() {
    // We do not assert the presence of block_99 (^b0), since it will be deleted by the transaltion for being empty.
    qoalahost.blk_meta  {block_id = "block_99", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.remote_id_ref  {classical = false, quantum = true, remote = @Bob}
    qoalahost.nop_term
    // CHECK: ^[[BLK_1:.*]] { type = QL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
    ^bb1:
        qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        // CHECK-NEXT: tuple<%[[REG_0:.*]]> = run_subroutine() : local_qubit_alloc_and_measure
        %0 = qoalahost.call @local_qubit_alloc_and_measure() : () -> i1
    // CHECK: ^[[BLK_2:.*]] { type = QC; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
    ^bb2:
        qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        // CHECK-NEXT: tuple<%[[REG_1:.*]]> = run_request() : entangle_measure
        %1 = qoalahost.call @entangle_measure() : () -> i1
  }
}
