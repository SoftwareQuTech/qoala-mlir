module {
    func.func @f() {
    ^b0:
        %0 = "qoala_lir.new_qubit"() : () -> f64
        %1 = "qoala_lir.constant"() {value = 2 : i32} : () -> i32

        %2 = "qoala_lir.multiply_const"(%1) {constant = 3 : i32} : (i32) -> i32
        %3 = "qoala_lir.conditional_multiply_const"(%2, %1) {constant = 3 : i32} : (i32, i32) -> i32

        cf.br ^b1

    ^b1:
        %q, %success = "qoala_lir.entangle_keep"() : () -> (f64, i1)
        cf.cond_br %success, ^b2, ^b1

    ^b2:
        return
    }
}