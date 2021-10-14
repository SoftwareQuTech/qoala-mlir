// %0 = "hir.gate" "double"(0.0) {} : (i8) -> i8
// %0 = hir.constant() {value = 0.0 : f64} : () -> f64

!my_type = type f32

%0 = "hir.new_value"() : () -> f32
%1 = "hir.new_value"() : () -> f32
"hir.one_operand"(%1) : (!my_type) -> ()
%q0 = "hir.qalloc"() : () -> f64