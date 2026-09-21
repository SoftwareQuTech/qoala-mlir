// RUN: qoala-opt %s --verify-diagnostics

module {
    qremote.remote @Bob
    // expected-error@+1 {{EPRS-related operations are not allowed in local routines}}
    netqasm.local_routine @invalid_local_routine() -> i32 {
        %vqubit = netqasm.qalloc : i32
        netqasm.eprs %vqubit {remote = @Bob}
        netqasm.return %vqubit : i32
    }

    qoalahost.main_func @no_module() {
        %0 = qoalahost.call @invalid_local_routine() : () -> i32
      ^bb1:
        qoalahost.return
    }
}