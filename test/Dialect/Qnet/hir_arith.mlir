// RUN: qoala-opt %s | FileCheck %s

module {
// CHECK: arith.constant
    %seven = arith.constant 7 : i32
// CHECK: arith.constant
    %pi = arith.constant 3.141592 : f32

// CHECK:       arith.constant
// CHECK-NEXT:  arith.constant
// CHECK-NEXT:  arith.addi
    %int_a = arith.constant 3 : i32
    %int_b = arith.constant 5 : i32
    %int_c = arith.addi %int_a, %int_b : i32

// CHECK:       arith.constant
// CHECK-NEXT:  arith.constant
// CHECK-NEXT:  arith.addf
    %float_a = arith.constant 1.23 : f32
    %float_b = arith.constant 4.56 : f32
    %float_c = arith.addf %float_a, %float_b : f32

// CHECK:       arith.fptosi
    %rounded = arith.fptosi %float_c : f32 to i32
// CHECK:       arith.muli
    %mul_i = arith.muli %int_a, %int_b : i32
// CHECK:       arith.mulf
    %mul_f = arith.mulf %float_a, %float_b : f32
// CHECK:       arith.ceildivsi
    %div_i_ceil = arith.ceildivsi %int_a, %int_b : i32
// CHECK:       arith.floordivsi
    %div_i_floor = arith.floordivsi %int_a, %int_b : i32
// CHECK:       arith.divf
    %div_f = arith.divf %float_a, %float_b : f32
}