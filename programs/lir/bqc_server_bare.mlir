"builtin.module"() ({
    "lir.remote_node"() {sym_name = "client"} : () -> ()

    "lir.subroutine"() ({
    ^b0(%q0: !lir.qubit, %q1: !lir.qubit):
        %q0a, %q1a = "lir.cphase"(%q0, %q1) : (!lir.qubit, !lir.qubit) -> (!lir.qubit, !lir.qubit)
        "lir.return"(%q0a, %q1a) : (!lir.qubit, !lir.qubit) -> ()
    }) {
        function_type = (!lir.qubit, !lir.qubit) -> (!lir.qubit, !lir.qubit),
        sym_name = "subrt1"
    } : () -> ()

    "lir.subroutine"() ({
    ^b0(%q: !lir.qubit, %delta: i32):
        %qa = "lir.rot_z"(%q, %delta) : (!lir.qubit, i32) -> !lir.qubit
        %qb = "lir.had"(%qa) : (!lir.qubit) -> !lir.qubit
        %m = "lir.measure"(%qb) : (!lir.qubit) -> i1
        "lir.return"(%m) : (i1) -> ()
    }) {
        function_type = (!lir.qubit, i32) -> i1,
        sym_name = "subrt2"
    } : () -> ()

    %q0 = "lir.entangle_keep"() {remote_node = @client} : () -> !lir.qubit
    %q1 = "lir.entangle_keep"() {remote_node = @client} : () -> !lir.qubit

    %q0a, %q1a = "lir.call"(%q0, %q1) {callee = @subrt1} : (!lir.qubit, !lir.qubit) -> (!lir.qubit, !lir.qubit)

    %delta1 = "lir.receive_msg"() {remote_node = @client} : () -> i32

    %m1 = "lir.call"(%q0a, %delta1) {callee = @subrt2} : (!lir.qubit, i32) -> i1
}) : () -> ()