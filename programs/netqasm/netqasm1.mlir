module {
    "netqasm.subroutine"() ({
        %0 = "netqasm.new_qubit"() : () -> f64
        %1 = "netqasm.constant"() {value = 2 : i32} : () -> i32
        "netqasm.return"(%1, %0) : (i32, f64) -> ()
    }) {sym_name = "subrt1", function_type = () -> ()} : () -> ()
}