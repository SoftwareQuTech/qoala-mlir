"builtin.module"() ({
    "func.func"()({
        // region #0
        %var0 = "arith.constant"() {value = 3: i32} : () -> i32
        "func.return"() : () -> ()
    }) 
    {
        // attributes
        function_type = () -> (),
        sym_name = "func1",
        sym_visibility="private"
    } : () -> ()

}) : () -> ()

// "builtin.module"() ({
//     "func.func"()({}) {"function_type"= () -> (), "sym_name" = "func1"} : () -> ()
//     // %var0 = "host.constant"() {value = 3: i32} : () -> i32
//     // ^b1:
//     // %var1 = "host.constant"() {value = 3: i32} : () -> i32
// }) : () -> ()

//     // %var1 = "host.constant"() {value = 3: i32} : () -> i32