// RUN: qoala-opt %s --verify-diagnostics

module {
    netqasm.local_routine @local_routine_1() -> i32 {
        %vq = netqasm.qalloc : i32
        netqasm.init %vq
        netqasm.return %vq : i32
    }

    qoalahost.main_func @no_module() {
        // expected-error@+1 {{Called function 'local_routine_unknown' was not found in the module.}}
        %0 = qoalahost.call @local_routine_unknown() : () -> i32
      ^bb1:
        qoalahost.return
    }
}