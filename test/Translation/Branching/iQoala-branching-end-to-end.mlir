// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s

// Python Program (file assumed test.py):
//from qoala import QoalaProgram
//from qoala.operations import Remote
//from qoala.types.classical import Int
//from qoala.types.quantum import Entangle
//from qoala.operations.communication import recv_int
//from qoala.operations.branching import if_cond
//from qoala.types.quantum.qubit import LocalQubit
//
//
//@QoalaProgram
//def teleport():
//    Remote("Alice")
//    qubit = Entangle("Alice")
//    local_qubit = LocalQubit()
//    x = recv_int("Alice")
//
//    with if_cond(x == 0) as (branch_true, branch_false):
//        with branch_true:
//            qubit.X()
//        with branch_false:
//            local_qubit.Z()
//
//    val = Int(10)
//
//
//if __name__ == "__main__":
//    _, module = teleport.compile()
//    print(str(module))

// Compiled like this to get the HIR below:
// python test.py > test-hir.ll
// qoala-opt --lower-qoala-hir-to-mir test-hir.ll > test-mir.ll
// qoala-opt --lower-qoala-mir-to-lir test-mir.ll > test-lir.ll

// In this example we will only assert the structure of the CFG
// CHECK: ^b6 { type = CL; predecessors = []; dependencies = [b1, b5]; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: beq(%2, %1) : b7
// CHECK-NEXT: jump() : b9

// CHECK: ^b7 { type = QL; predecessors = [b6]; dependencies = [b2]; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: run_subroutine() : __qoala_wrapper2

// CHECK: ^b8 { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: jump() : b11

// CHECK: ^b9 { type = QL; predecessors = [b6]; dependencies = [b3]; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: run_subroutine() : __qoala_wrapper3

// CHECK: ^b10 { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: jump() : b11

// CHECK:^b11 { type = CL; predecessors = [b10, b8]; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:

module {
  qremote.remote @Alice
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.request_routine @__qoala_wrapper0() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0 {remote = @Alice}
    netqasm.return %0 : i32
  }
  netqasm.local_routine @__qoala_wrapper1() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.local_routine @__qoala_wrapper2(%arg0: i32) {
    netqasm.x %arg0
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper3(%arg0: i32) {
    netqasm.z %arg0
    netqasm.return
  }
  qoalahost.main_func @teleport() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.remote_id_ref  {classical = true, quantum = true, remote = @Alice}
    qoalahost.nop_term
  ^bb1:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %c0_i32 = arith.constant 0 : i32
    qoalahost.nop_term
  ^bb2:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @__qoala_wrapper0() : () -> i32
  ^bb3:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = qoalahost.call @__qoala_wrapper1() : () -> i32
  ^bb4:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    qoalahost.nop_term
  ^bb5:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = ["block_0"], predecessors = [], prev_comm = "", prev_ent = ""}
    %2 = qoalahost.recv_int  {remote = @Alice} : i32
  ^bb6:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_6", deadlines = {}, dependencies = ["block_1", "block_5"], predecessors = [], prev_comm = "", prev_ent = ""}
    %3 = arith.cmpi eq, %2, %c0_i32 : i32
    cf.cond_br %3, ^bb7, ^bb9
  ^bb7:  // pred: ^bb6
    qoalahost.blk_meta  {block_id = "block_7", deadlines = {}, dependencies = ["block_2"], predecessors = ["block_6"], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper2(%0) : (i32) -> ()
  ^bb8:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_8", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    cf.br ^bb11
  ^bb9:  // pred: ^bb6
    qoalahost.blk_meta  {block_id = "block_9", deadlines = {}, dependencies = ["block_3"], predecessors = ["block_6"], prev_comm = "", prev_ent = ""}
    qoalahost.call @__qoala_wrapper3(%1) : (i32) -> ()
  ^bb10:  // no predecessors
    qoalahost.blk_meta  {block_id = "block_10", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    cf.br ^bb11
  ^bb11:  // 2 preds: ^bb8, ^bb10
    qoalahost.blk_meta  {block_id = "block_11", deadlines = {}, dependencies = [], predecessors = ["block_10", "block_8"], prev_comm = "", prev_ent = ""}
    qoalahost.return
  }
}
