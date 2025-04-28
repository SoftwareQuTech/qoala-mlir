// RUN: qoala-translate %s --mlir-to-iqoala --verify-diagnostics
// This test check that the conversion fails, due to a request routine using its argument
// We need to add the "--verify-diagnostics" option to check for the error
module {
  qremote.remote @Bob
  // CHECK-LABEL: __qoala_convert_float_angle
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  // expected-error@+1 {{argument #0 from request routine '__qoala_wrapper0' is used. This is not supported}}
  netqasm.request_routine @__qoala_wrapper0(%arg: i32) -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0  {remote = @Bob}
    netqasm.return %arg : i32
  }
  qoalahost.main_func @test_call_local_routine() {
    qoalahost.blk_meta  {block_id = "block_0", predecessors = []}
    %cst = arith.constant 0 : i32
    %0 = qoalahost.call @__qoala_wrapper0(%cst) : (i32) -> i32
    ^bb1:
      qoalahost.blk_meta  {block_id = "block_1", predecessors = []}
      qoalahost.return
  }
}
