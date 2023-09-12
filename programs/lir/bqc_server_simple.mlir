module {
    "lir.remote_node"() {sym_name = "client"} : () -> ()

    %e0 = "lir.entangle_keep"() {remote_node = @client} : () -> !lir.equbit
    %e1 = "lir.entangle_keep"() {remote_node = @client} : () -> !lir.equbit

    %q0 = "lir.ent_to_local"(%e0) : (!lir.equbit) -> !lir.qubit
    %q1 = "lir.ent_to_local"(%e1) : (!lir.equbit) -> !lir.qubit

    %q0a, %q1a = "lir.cphase"(%q0, %q1) : (!lir.qubit, !lir.qubit) -> (!lir.qubit, !lir.qubit)

    %delta1 = "lir.receive_msg"() {remote_node = @client} : () -> i32

    %q1b = "lir.rot_z"(%q1a, %delta1) : (!lir.qubit, i32) -> !lir.qubit
    %q1c = "lir.had"(%q1b) : (!lir.qubit) -> !lir.qubit
    %m1 = "lir.measure"(%q1c) : (!lir.qubit) -> i1
}