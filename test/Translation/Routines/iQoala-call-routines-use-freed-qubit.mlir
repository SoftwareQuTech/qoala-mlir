// RUN: qoala-translate %s --mlir-to-iqoala --verify-diagnostics

// Note: According to the lifetime of a qubit, in LIR a "qubit value" can be returned either when:
// (a) when using netqasm.eprs in a request routine.
// (b) when using netqasm.qalloc + netqasm.qinit in a loca routine.
// There are two cases in LIR that *cannot* return qubit values to the qoalahost section:
// 1. when a local routine uses netqasm.qalloc + netqasm.qinit and then netqasm.measure *within the
//    same local routine*, and
// 2. when a request routine uses netqasm.eprs_measure.
// In these two cases, the lifetime of a qubit is "local" to the routine (either local or request)
// and that (qubit) value is *not* returned back to the qoalahost section.
// These two special cases are tested in iQoala-call-routines-use-qubit-scope.mlir

module {
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @local_qubit() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.local_routine @bsm_h(%0: i32) {
    netqasm.hadamard %0
    netqasm.return
  }
  netqasm.local_routine @rotate(%vqubit: i32, %zero_arg: i32, %four_arg: i32) {
    netqasm.rot_x %vqubit, %zero_arg, %four_arg
    netqasm.return
  }
  netqasm.local_routine @bsm_free_local(%0: i32) {
    netqasm.qfree %0
    netqasm.return
  }
  // expected-error@+1 {{qoalahost.main_func' op cannot convert a block inside function 'test_reordering_teleport'}}
  qoalahost.main_func @test_reordering_teleport() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %zero_i32 = arith.constant 0 : i32
    %four_i32 = arith.constant 4 : i32
    %0 = qoalahost.call @local_qubit() : () -> i32
    ^bb2:
        qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.call @bsm_h(%0) : (i32) -> ()
    ^bb4:
        qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = ["block_1"], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.call @bsm_free_local(%0) : (i32) -> i1
    ^bb5:
        qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
        // This line should yield a translation error. Despite the MLIR value still "exists", the qubit was free's
        // by the last call. Whichmeans that it is not usable anymore.
        // expected-error@+2 {{'qoalahost.call' op makes use of an already freed or measured qubit.}}
        // expected-error@+1 {{'qoalahost.call' op cannot convert operation '%4 = "qoalahost.call"(%2, %0, %1) <{callee = @rotate}> : (i32, i32, i32) -> i1'}}
        qoalahost.call @rotate(%0, %zero_i32, %four_i32) : (i32, i32, i32) -> i1
  }
}
