"builtin.module"() ({
    "netqasm.remote_node"() {"sym_name" = "client"} : () -> ()

    "func.func"() ({
        %q0 = "netqasm.entangle_keep"() {remote_node = @client} : () -> !netqasm.equbit
        %q1 = "netqasm.entangle_keep"() {remote_node = @client} : () -> !netqasm.equbit
        "func.return"(%q0, %q1) : (!netqasm.equbit, !netqasm.equbit) -> ()
    }) {
        sym_name = "rsp",
        function_type = () -> (!netqasm.equbit, !netqasm.equbit)
    } : () -> ()

    %e0, %e1 = "func.call"() {callee = @rsp} : () -> (!netqasm.equbit, !netqasm.equbit)
    %q0 = "netqasm.ent_to_local"(%e0) : (!netqasm.equbit) -> !netqasm.qubit
    %q1 = "netqasm.ent_to_local"(%e1) : (!netqasm.equbit) -> !netqasm.qubit

    %q0a, %q1a = "netqasm.cphase"(%q0, %q1) : (!netqasm.qubit, !netqasm.qubit) -> (!netqasm.qubit, !netqasm.qubit)

    %delta1 = "netqasm.receive_msg"() {remote_node = @client} : () -> i32

    %q1b = "netqasm.rot_z"(%q1a, %delta1) : (!netqasm.qubit, i32) -> !netqasm.qubit
    %q1c = "netqasm.had"(%q1b) : (!netqasm.qubit) -> !netqasm.qubit
    %m1 = "netqasm.measure"(%q1c) : (!netqasm.qubit) -> i1
}) : () -> ()