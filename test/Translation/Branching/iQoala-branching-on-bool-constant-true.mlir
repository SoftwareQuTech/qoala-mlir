// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s

// CHECK: META_START
// CHECK-NEXT: name: test_branching_unconditional
// CHECK-NEXT: parameters:
// CHECK-NEXT: csockets:
// CHECK-NEXT: epr_sockets:
// CHECK-NEXT: META_END
// CHECK-NEXT: ^b0 { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: %[[HOST_REG0:.*]] = assign_cval() : 3
// CHECK-NEXT: %[[HOST_REG1:.*]] = assign_cval() : 5
// CHECK-NEXT: jump() : b1
// CHECK: ^b1 { type = CL; predecessors = [b0]; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: jump() : b2
// CHECK: ^b2 { type = CL; predecessors = [b1]; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: jump() : b3
// CHECK: ^b3 { type = QL; predecessors = [b2]; dependencies = [b0]; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: tuple<%[[HOST_REG2:.*]]> = run_subroutine(tuple<%[[HOST_REG0]]>) : __qoala_wrapper0
// CHECK: ^b4 { type = CL; predecessors = []; dependencies = [b0, b3]; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: %[[HOST_REG3:.*]] = add_cval_c(%[[HOST_REG2]], %[[HOST_REG1]])

// CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params: p0
// CHECK-NEXT: returns: m0
// CHECK-NEXT: uses:
// CHECK-NEXT: keeps:
// CHECK-NEXT: request:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: load R[[ARG0_REG:.*]] @input[0]
// CHECK-NEXT: set C[[C_REG1:.*]] 10
// CHECK-NEXT: jmp 3
// CHECK-NEXT: add C[[C_REG2:.*]] R[[ARG0_REG]] C[[C_REG1]]
// CHECK-NEXT: jmp 3
// CHECK-NEXT: sub C[[C_REG3:.*]] R[[ARG0_REG]] C[[C_REG1]]
// CHECK-NEXT: jmp -3
// CHECK-NEXT: mul C[[C_REG4:.*]] R[[ARG0_REG]] C[[C_REG1]]
// CHECK-NEXT: store C[[C_REG4]] @output[0]
// CHECK-NEXT: NETQASM_END

module {
  qremote.remote @server
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> i1
  netqasm.local_routine @__qoala_wrapper0(%arg0: i32) -> i32 {
    %cstA = arith.constant 10 : i32
    %true = arith.constant true
    cf.cond_br %true, ^bb1, ^bb2
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
  qoalahost.main_func @test_branching_unconditional() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.remote_id_ref  {classical = true, quantum = false, remote = @server}
    qoalahost.nop_term
  ^bb0:
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %cstA = arith.constant 3 : i32
    %cstB = arith.constant 5 : i32
    %true = arith.constant true
    cf.cond_br %true, ^bb1, ^bb2
  ^bb1:
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = ["block_1"], prev_comm = "", prev_ent = ""}
    qoalahost.send_int %cstA {remote = @server} : i32
    cf.br ^bb3
  ^bb2:
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = [], predecessors = ["block_2"], prev_comm = "", prev_ent = ""}
    qoalahost.send_int %cstB {remote = @server} : i32
    cf.br ^bb3
  ^bb3:
    qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = ["block_1"], predecessors = ["block_3"], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @__qoala_wrapper0(%cstA) : (i32) -> i32
  ^bb4:
     qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = ["block_1", "block_4"], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = arith.addi %0, %cstB : i32
    qoalahost.return %1 : i32
  }
}
