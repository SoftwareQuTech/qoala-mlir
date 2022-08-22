module {
    func.func @f() {
    ^b0:
        %0 = "qoala_hir.new_qubit"() : () -> f64
        %1 = "qoala_hir.constant"() {value = 2 : i32} : () -> i32

        %2 = "qoala_hir.multiply_const"(%1) {constant = 3 : i32} : (i32) -> i32
        %3 = "qoala_hir.conditional_multiply_const"(%2, %1) {constant = 3 : i32} : (i32, i32) -> i32

        %s = "qoala_hir.int_struct"() : () -> !qoala_hir.intstruct
        "qoala_hir.use_int_struct"(%s) : (!qoala_hir.intstruct) -> ()

        cf.br ^b1

    ^b1:
        %q, %success = "qoala_hir.entangle_keep"() : () -> (f64, i1)
        cf.cond_br %success, ^b2, ^b1

    ^b2:
        return
    }
}