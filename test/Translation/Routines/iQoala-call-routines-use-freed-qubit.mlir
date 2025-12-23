// RUN: qoala-translate %s --mlir-to-iqoala --verify-diagnostics

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
  netqasm.local_routine @rotate(%vqubit: i32) {
    netqasm.rot_x %vqubit (0 : ui32, 4 : ui32)
    netqasm.return
  }
  netqasm.local_routine @bsm_free_local(%0: i32) {
    netqasm.qfree %0
    netqasm.return
  }
  // expected-error@+1 {{qoalahost.main_func' op cannot convert a block inside function 'test_reordering_teleport'}}
  qoalahost.main_func @test_reordering_teleport() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
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
        // expected-error@+1 {{'qoalahost.call' op cannot convert operation '%2 = "qoalahost.call"(%0) <{callee = @rotate}> : (i32) -> i1'}}
        qoalahost.call @rotate(%0) : (i32) -> i1
  }
}
