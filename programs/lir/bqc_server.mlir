module {
    "lir.remote_node"() {sym_name = "client"} : () -> ()

    lir.subroutine @subrt1(%q0: !lir.qubit, %q1: !lir.qubit) -> (!lir.qubit, !lir.qubit) {
        %q0a, %q1a = "lir.cphase"(%q0, %q1) : (!lir.qubit, !lir.qubit) -> (!lir.qubit, !lir.qubit)
        lir.return %q0a, %q1a : !lir.qubit, !lir.qubit
    }

    lir.subroutine @subrt2(%q: !lir.qubit, %delta: i32) -> i1 {
        %qa = lir.rot_z %q, %delta
        %qb = lir.had %qa
        %m = lir.measure %qb
        lir.return %m : i1
    }

    %q0 = lir.entangle_keep @client
    %q1 = lir.entangle_keep @client

    %q0a, %q1a = lir.call @subrt1(%q0, %q1) : (!lir.qubit, !lir.qubit) -> (!lir.qubit, !lir.qubit)

    %delta1 = lir.receive_msg @client : i32

    %m1 = lir.call @subrt2(%q0a, %delta1) : (!lir.qubit, i32) -> i1
}