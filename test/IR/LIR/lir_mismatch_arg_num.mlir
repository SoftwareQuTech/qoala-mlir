// RUN: qoala-opt %s --verify-diagnostics

module {
    netqasm.local_routine @local_routine_1(%unused: i32) -> i32 {
        %vq = netqasm.qalloc : i32
        netqasm.init %vq
        netqasm.return %vq : i32
    }

    qoalahost.main_func @no_module() {
        // expected-error@+1 {{Call operation does not match the number of arguments of the callee.}}
        %0 = qoalahost.call @local_routine_1() : () -> i32
      ^bb1:
        qoalahost.return
    }
}