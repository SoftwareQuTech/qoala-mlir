module {
    "netqasm.subroutine"() ({
        %0 = "netqasm.new_comm_qubit"() : () -> !netqasm.comm_qubit
        %1 = "netqasm.constant"() {value = 2 : i32} : () -> i32
        "netqasm.return"(%1, %0) : (i32, !netqasm.comm_qubit) -> ()
    }) {sym_name = "subrt1", function_type = () -> ()} : () -> ()
}