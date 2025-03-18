// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s

// This test is a bit fragile. In the QoalaHost section it is not possible
// to capture the name of the block to branch to, since it is usually defined
// AFTER the place where it is used.
// This is why all the branching destinations are "hard-coded"

// CHECK: META START
// CHECK-NEXT: name: test_branching
// CHECK-NEXT: parameters: Bob
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META END
// CHECK-NEXT: b0 { type = CL }
// CHECK-NEXT: %[[HOST_REG0:.*]] = assign_cval () : 3
// CHECK-NEXT: %[[HOST_REG1:.*]] = assign_cval () : 2
// CHECK-NEXT: bne (%[[HOST_REG0]], %[[HOST_REG1]]) : b1
// CHECK-NEXT: jump () : b2
// CHECK: b1 { type = CL }
// CHECK-NEXT: jump () : b3
// CHECK: b2 { type = CL }
// CHECK-NEXT: jump () : b3
// CHECK: b3 { type = CL }
// CHECK-NEXT: %[[HOST_REG2:.*]] = add_cval_c (%[[HOST_REG0:.*]], %[[HOST_REG1:.*]])

// CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params: p0
// CHECK-NEXT: returns: m0
// CHECK-NEXT: uses:
// CHECK-NEXT: keeps:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set C[[C_REG0:.*]] 0
// CHECK-NEXT: load R[[ARG0_REG:.*]] @input[C[[C_REG0]]]
// CHECK-NEXT: set C[[C_REG1:.*]] 10
// CHECK-NEXT: bne (C[[C_REG1]], R[[ARG0_REG]]) : 1
// CHECK-NEXT: jump 3
// CHECK-NEXT: add C[[C_REG2:.*]] C[[ARG0_REG]] C[[C_REG1]]
// CHECK-NEXT: jump 3
// CHECK-NEXT: sub C[[C_REG3:.*]] C[[ARG0_REG]] C[[C_REG1]]
// CHECK-NEXT: jump -3
// CHECK-NEXT: mul C[[C_REG4:.*]] C[[ARG0_REG]] C[[C_REG1]]
// CHECK-NEXT: store C[[C_REG4]] @output[0]
// CHECK-NEXT: NETQASM_END

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> i1
  netqasm.local_routine @__qoala_wrapper0(%arg0: i32) -> i32 {
    %cstA = arith.constant 10 : i32
    %jump_loc = arith.cmpi ne, %cstA, %arg0 : i32
    cf.cond_br %jump_loc, ^bb1, ^bb2
  ^bb1:
    %1 = arith.addi %arg0, %cstA : i32
    cf.br ^bb3
  ^bb2:
    %2 = arith.subi %arg0, %cstA : i32
    cf.br ^bb1
  ^bb3:
    %3 = arith.muli %arg0, %cstA : i32
    netqasm.return %3 : i32
  }
  qoalahost.main_func @test_branching() {
    %cstA = arith.constant 3 : i32
    %cstB = arith.constant 2 : i32
    %jump = arith.cmpi ne, %cstA, %cstB : i32
    cf.cond_br %jump, ^bb0, ^bb1
  ^bb0:
    cf.br ^bb2
  ^bb1:
    cf.br ^bb2
  ^bb2:
    %0 = qoalahost.call @__qoala_wrapper0(%cstA) : (i32) -> i32
    %1 = arith.addi %cstA, %cstB : i32
    qoalahost.return
  }
}
