// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META_START
// CHECK-NEXT: name: test_arith_operations
// CHECK-NEXT: parameters: Bob_id
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META_END
// CHECK-NEXT: ^b[[BLOCK0:.*]] { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: %[[HOST_REG0:.*]] = assign_cval() : 3
// CHECK-NEXT: %[[HOST_REG1:.*]] = assign_cval() : 2
// CHECK-NEXT: %[[HOST_REG2:.*]] = add_cval_c(%[[HOST_REG0]], %[[HOST_REG1]])
// CHECK-NEXT: %[[HOST_REG3:.*]] = sub_cval_c(%[[HOST_REG0]], %[[HOST_REG1]])
// The compiler will check in order which one of the operands is a constant
// whose value can be "forwarded". Being that said, HOST_REG0 is the first operand
// which is a constant and can be forwarded:
// CHECK-NEXT: %[[HOST_REG4:.*]] = mult_const(%[[HOST_REG1]]) : 3
// Instructions quot and rem not supported in the QoalaHost section in qoala-sim yet
// %[[HOST_REG4:.*]] = quot(%[[HOST_REG0]], %[[HOST_REG1]])
// %[[HOST_REG4:.*]] = rem(%[[HOST_REG0]], %[[HOST_REG1]])
// CHECK: ^b[[BLOCK1:.*]] { type = QL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: run_subroutine() : __qoala_wrapper0

//CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params:
// CHECK-NEXT: returns:
// CHECK-NEXT: uses:
// CHECK-NEXT: keeps:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set C[[C_REG0:.*]] 10
// CHECK-NEXT: set C[[C_REG1:.*]] 15
// CHECK-NEXT: add C[[C_REG2:.*]] C[[C_REG0:.*]] C[[C_REG1:.*]]
// CHECK-NEXT: sub C[[C_REG3:.*]] C[[C_REG0:.*]] C[[C_REG1:.*]]
// CHECK-NEXT: mul C[[C_REG4:.*]] C[[C_REG0:.*]] C[[C_REG1:.*]]
// CHECK-NEXT: div C[[C_REG5:.*]] C[[C_REG0:.*]] C[[C_REG1:.*]]
// CHECK-NEXT: rem C[[C_REG5:.*]] C[[C_REG0:.*]] C[[C_REG1:.*]]
// CHECK-NEXT: NETQASM_END

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @__qoala_wrapper0() -> () {
    %cstA = arith.constant 10 : i32
    %cstB = arith.constant 15 : i32
    %resA = arith.addi %cstA, %cstB : i32
    %resB = arith.subi %cstA, %cstB : i32
    %resC = arith.muli %cstA, %cstB : i32
    %resD = arith.divui %cstA, %cstB : i32
    %resE = arith.remui %cstA, %cstB : i32
    netqasm.return
  }
  qoalahost.main_func @test_arith_operations() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %cstA = arith.constant 3 : i32
    %cstB = arith.constant 2 : i32
    %resA = arith.addi %cstA, %cstB : i32
    %resB = arith.subi %cstA, %cstB : i32
    %resC = arith.muli %cstA, %cstB : i32
    qoalahost.nop_term
  ^bb1:
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper0() : () -> ()
  ^bb2:
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.return
  }
}
