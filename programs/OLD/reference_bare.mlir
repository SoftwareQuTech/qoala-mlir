"builtin.module"() ({
    "func.func"() ({
        %0 = "arith.constant"() {value = 42: i32} : () -> i32
        %1 = "arith.constant"() {value = 3.14: f64} : () -> f64
        "func.return"(%0, %1) : (i32, f64) -> ()
    }) {
        sym_name = "subrt1",
        function_type = () -> (i32, f64)
    } : () -> ()

    %res0, %res1 = "func.call"() {callee = @subrt1} : () -> (i32, f64)
}) {} : () -> ()
