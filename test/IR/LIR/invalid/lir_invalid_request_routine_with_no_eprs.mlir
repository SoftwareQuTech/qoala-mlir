// RUN: qoala-opt %s --verify-diagnostics

module {
    qremote.remote @Bob
    // expected-error@+1 {{Request routines must contain an EPRS-related operation}}
    netqasm.request_routine @invalid_local_routine() -> i32 {
        %vqubit = netqasm.qalloc : i32
        netqasm.init %vqubit
        netqasm.return %vqubit : i32
    }

    qoalahost.main_func @no_module() {
        %0 = qoalahost.call @invalid_local_routine() : () -> i32
      ^bb1:
        qoalahost.return
    }
}