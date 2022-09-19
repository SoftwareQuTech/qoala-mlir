module {
    "qoala_lir_m.netqasmfunc"() ({
        %0 = "qoala_lir_m.new_qubit"() : () -> f64
        %1 = "qoala_lir_m.constant"() {value = 2 : i32} : () -> i32
        "qoala_lir_m.netqasm_return"(%1, %0) : (i32, f64) -> ()
    }) {sym_name = "subrt1", function_type = () -> ()} : () -> ()

    "qoala_lir_m.netqasmfunc"() ({
    ^b0(%q: !qoala_lir_m.comm_qubit):
        %0 = "qoala_lir_m.new_qubit"() : () -> f64
        %1 = "qoala_lir_m.constant"() {value = 2 : i32} : () -> i32
        "qoala_lir_m.netqasm_return"(%1, %0) : (i32, f64) -> ()
    }) {
        sym_name = "subrt2",
        function_type = (!qoala_lir_m.comm_qubit) -> ()
    } : () -> ()

    func.func @f() {
    ^b0:
        %0 = "qoala_lir_m.new_qubit"() : () -> f64
        %1 = "qoala_lir_m.constant"() {value = 2 : i32} : () -> i32

        %2 = "qoala_lir_m.multiply_const"(%1) {constant = 3 : i32} : (i32) -> i32
        %3 = "qoala_lir_m.conditional_multiply_const"(%2, %1) {constant = 3 : i32} : (i32, i32) -> i32

        %comm = "qoala_lir_m.new_comm_qubit"() : () -> !qoala_lir_m.comm_qubit
        "qoala_lir_m.generic_call"() {callee = @subrt1} : () -> ()
        "qoala_lir_m.generic_call"(%comm) {callee = @subrt2} : (!qoala_lir_m.comm_qubit) -> ()

        "cf.br"() [^b1] : () -> ()

    ^b1:
        %q, %success = "qoala_lir_m.entangle_keep"() : () -> (f64, i1)
        cf.cond_br %success, ^b2, ^b1

    ^b2:
        return
    }
}