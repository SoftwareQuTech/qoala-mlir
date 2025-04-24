// RUN: qoala-opt %s --verify-diagnostics

module {
    netqasm.local_routine @local_routine_1(%unused: i32, %unused_B: i1, %unused_C: i32) -> i32 {
        %vq = netqasm.qalloc : i32
        netqasm.init %vq
        netqasm.return %vq : i32
    }

    qoalahost.main_func @no_module() {
        %cst = arith.constant 1 : i32
        %cst_B = arith.constant 10 : i32
        %cst_C = arith.constant 5 : i32
        // expected-error@+1 {{Operand #1 does not match the type of the callee.}}
        %0 = qoalahost.call @local_routine_1(%cst, %cst_B, %cst_C) : (i32, i32, i32) -> i32
      ^bb1:
        qoalahost.return
    }
}