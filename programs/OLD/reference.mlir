module {
    func.func @subrt1() -> (i32, f64) {
        %0 = "arith.constant"() {value = 42: i32} : () -> i32
        %1 = "arith.constant"() {value = 3.14: f64} : () -> f64
        func.return %0, %1 : i32, f64
    }

    %0, %1 = func.call @subrt1() : () -> (i32, f64)
}