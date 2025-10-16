// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META_START
// CHECK-NEXT: name: test_local_routine_args
// CHECK-NEXT: parameters: Bob_id
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META_END
// CHECK-NEXT: ^b[[BLOCK0:.*]] { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: %[[HOST_REG0:.*]] = assign_cval() : 3
// CHECK-NEXT: %[[HOST_REG1:.*]] = assign_cval() : 5
// CHECK-NEXT: %[[HOST_REG2:.*]] = assign_cval() : 2
// CHECK: ^b[[BLOCK1:.*]] { type = QL; predecessors = []; dependencies = [b0]; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: run_subroutine(tuple<%[[HOST_REG0]]; %[[HOST_REG1]]; %[[HOST_REG2]]>) : __qoala_wrapper0

//CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params: p0, p1, p2
// CHECK-NEXT: returns: {{[[:space:]]}}
// CHECK-SAME: uses: {{[[:space:]]}}
// CHECK-SAME: keeps: {{[[:space:]]}}
// CHECK-SAME: NETQASM_START
// CHECK-NEXT: load R[[R_REG0:.*]] @input[0]
// CHECK-NEXT: load R[[R_REG1:.*]] @input[1]
// CHECK-NEXT: load R[[R_REG2:.*]] @input[2]
// CHECK-NEXT: set C[[C_REG3:.*]] 25
// CHECK-NEXT: NETQASM_END

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.local_routine @__qoala_wrapper0(%arg0: i32, %arg1: i32, %arg2: i32) -> () {
    %cst = arith.constant 25 : i32
    netqasm.return
  }
  qoalahost.main_func @test_local_routine_args() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %cstA = arith.constant 3 : i32
    %cstB = arith.constant 5 : i32
    %cstC = arith.constant 2 : i32
    qoalahost.nop_term
  ^bb1:
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper0(%cstA, %cstB, %cstC) : (i32, i32, i32) -> ()
  ^bb2:
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.return
  }
}
