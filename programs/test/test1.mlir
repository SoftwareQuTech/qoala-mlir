// "builtin.module"() ({
//     "func.func"()({
//         // region #0
//         // block 0
//         %var0 = "arith.constant"() {value = 3: i32} : () -> i32
//         "cf.br"() [^bb0] : () -> ()
//         ^bb0:
//         "func.return"() : () -> ()
//     }) 
//     {
//         // attributes
//         function_type = () -> (),
//         sym_name = "func1",
//         sym_visibility="public"
//     } : () -> ()

// }) : () -> ()

// "builtin.module"() ({
//     "func.func"()({}) {"function_type"= () -> (), "sym_name" = "func1"} : () -> ()
//     // %var0 = "host.constant"() {value = 3: i32} : () -> i32
//     // ^b1:
//     // %var1 = "host.constant"() {value = 3: i32} : () -> i32
// }) : () -> ()

//     // %var1 = "host.constant"() {value = 3: i32} : () -> i32