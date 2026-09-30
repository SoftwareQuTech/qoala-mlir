// RUN: qoala-opt %s --verify-diagnostics

module {
    qremote.remote @Bob
    netqasm.request_routine @invalid_request_routine() -> i32 {
        %vqubit = netqasm.qalloc : i32
        netqasm.eprs %vqubit {remote = @Bob}
        %cst = arith.constant 10 : i32
        // expected-error@+1 {{Returned value '%1 = "arith.constant"() <{value = 10 : i32}> : () -> i32' is an i32 and it was not created by netqasm.qalloc operation.}}
        netqasm.return %cst : i32
    }

    qoalahost.main_func @no_module() {
        qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        %0 = qoalahost.call @invalid_request_routine() : () -> i32
      ^bb1:
        qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
        qoalahost.return
    }
}
