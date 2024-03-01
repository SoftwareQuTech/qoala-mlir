module {
    "netqasm.subroutine"() ({
        %0 = "netqasm.new_comm_qubit"() : () -> !netqasm.cqubit
        %e, %success = "netqasm.entangle_keep"() : () -> (!netqasm.equbit, i1)
        %1 = "netqasm.constant"() {value = 2 : i32} : () -> i32
        "netqasm.return"(%1, %0) : (i32, !netqasm.cqubit) -> ()
    }) {
        sym_name = "subrt1",
        function_type = () -> (i32, !netqasm.cqubit)
    } : () -> ()

    %success, %qubit = "netqasm.call"() {callee = @subrt1} : () -> (i32, !netqasm.cqubit)
}