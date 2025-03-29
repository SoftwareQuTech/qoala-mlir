// RUN: qoala-translate %s --mlir-to-iqoala --verify-diagnostics

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  // expected-error@+1 {{request routine '__qoala_wrapper0' must return a value}}
  netqasm.request_routine @__qoala_wrapper0() -> () {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0  {remote = @Bob}
    netqasm.return
  }
  qoalahost.main_func @test_call_request_routine() {
    // Note: there is an implicit "^bb0" not-rendered block declaration here
    // so this "call" operation is the one and only operation of the
    // first block of the main function
    qoalahost.call @__qoala_wrapper0() : () -> ()
    ^bb1:
        qoalahost.return
  }
}
