// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s

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
    netqasm.return %2: i1
  }
  qoalahost.main_func @test_removed_dependency() {
    // We do not assert the presence of block_99 (^b0), since it will be deleted by the transaltion for being empty.
    qoalahost.blk_meta  {block_id = "block_99", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.remote_id_ref  {classical = false, quantum = true, remote = @Bob}
    qoalahost.nop_term

    // For some reason, there is a single dependency on "block_99". We don't expect it in the final iQoala, since the
    // translate tool should remove this reference, since block_99 will be empty after translation
    // CHECK: ^[[BLK_1:.*]] { type = QL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
    ^bb1:
        qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = ["block_99"], predecessors = [], prev_comm = "", prev_ent = ""}
        %0 = qoalahost.call @local_qubit_alloc_and_measure() : () -> i1

    // For some reason, there is a dependency on "block_99" (together with a valid one). We don't expect the reference to
    // "block_99" in the final iQoala, since the translate tool should remove this reference, since block_99 will be
    // empty after translation
    // CHECK: ^[[BLK_2:.*]] { type = QC; predecessors = []; dependencies = ["[[BLK_1]]"]; prev_comm = ; prev_ent = ; deadlines = [] }:
    ^bb2:
        qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = ["block_1", "block_99"], predecessors = [], prev_comm = "", prev_ent = ""}
        %1 = qoalahost.call @entangle_measure() : () -> i1

    // Same test for predecessors list
    // CHECK: ^[[BLK_3:.*]] { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
    ^bb3:
        qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = [], predecessors = ["block_99"], prev_comm = "", prev_ent = ""}
        %2 = arith.constant 4 : i32
        qoalahost.nop_term

    // CHECK: ^[[BLK_4:.*]] { type = CL; predecessors = ["[[BLK_1]]"]; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
    ^bb4:
        qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = [], predecessors = ["block_1", "block_99"], prev_comm = "", prev_ent = ""}
        %3 = arith.constant 8 : i32
        qoalahost.nop_term

    // Same test for prev_comm and prev_ent
    // CHECK: ^[[BLK_5:.*]] { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
    ^bb5:
        qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "block_99", prev_ent = ""}
        %4 = arith.constant 12 : i32
        qoalahost.nop_term

    // CHECK: ^[[BLK_6:.*]] { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
    ^bb6:
        qoalahost.blk_meta  {block_id = "block_6", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = "block_99"}
        %5 = arith.constant 16 : i32
        qoalahost.nop_term
    // TODO - How can we test this for deadlines?
  }
}