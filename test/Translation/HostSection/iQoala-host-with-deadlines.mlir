// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META_START
// CHECK-NEXT: name: test_qubit_tracking
// CHECK-NEXT: parameters: Bob_id
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META_END
// CHECK-NEXT: ^b[[BLOCK0:.*]] { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: %[[HOST_REG0:.*]] = assign_cval() : 0
// CHECK: ^b[[BLOCK1:.*]] { type = QL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [b0: 100] }
// This call does not yield a result, because __qoala_wrapper0 "uses 0" and "keeps 0"
// CHECK-NEXT: run_subroutine() : __qoala_wrapper0
// CHECK: ^b[[BLOCK2:.*]] { type = QL; predecessors = []; dependencies = [b1]; prev_comm = ; prev_ent = ; deadlines = [b0: 500, b1: 300] }
// This call does not required an argument, since __qoala_wrapper "uses 0"
// CHECK-NEXT: %[[HOST_REG2:.*]] = run_subroutine() : __qoala_wrapper1
// CHECK: ^b[[BLOCK3:.*]] { type = CL; predecessors = []; dependencies = [b0, b2]; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: %[[HOST_REG3:.*]] = add_cval_c(%[[HOST_REG0]], %[[HOST_REG2]])

// CHECK: SUBROUTINE __qoala_wrapper0
// CHECK-NEXT: params: {{[[:space:]]}}
// CHECK-SAME: returns:  {{[[:space:]]}}
// CHECK-SAME: uses: [[QUBIT0:.*]]
// CHECK-NEXT: keeps: [[QUBIT0]]
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set [[QUBIT_REG0:.*]] [[QUBIT0]]
// CHECK-NEXT: init [[QUBIT_REG0]]
// CHECK-NEXT: NETQASM_END

// CHECK: SUBROUTINE __qoala_wrapper1
// Used qubits should not be marked as parameters
// CHECK-NEXT: params: {{[[:space:]]}}
// CHECK-SAME: returns: m0
// CHECK-NEXT: uses: [[QUBIT0]]
// CHECK-NEXT: keeps: {{[[:space:]]}}
// CHECK-SAME: NETQASM_START
// CHECK-NEXT: set [[QUBIT_REG0:.*]] [[QUBIT0]]
// CHECK-NEXT: meas [[QUBIT_REG0]] [[M_REG0:.*]]
// CHECK-NEXT: store [[M_REG0]] @output[0]
// CHECK-NEXT: NETQASM_END

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  // This routine returns directly a qubit "pointer"
  netqasm.local_routine @__qoala_wrapper0() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  // This routine returns directly a qubit measurement<
  netqasm.local_routine @__qoala_wrapper1(%qubit: i32) -> i1 {
    %0 = netqasm.measure %qubit : i1
    netqasm.return %0 : i1
  }
  qoalahost.main_func @test_qubit_tracking() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %cst = arith.constant 0 : i1
    qoalahost.nop_term
  ^bb1:
    qoalahost.blk_meta  {block_id = "block_1", deadlines = {block_0 = 100 : i64}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %0 = qoalahost.call @__qoala_wrapper0() : () -> i32
  ^bb2:
    qoalahost.blk_meta  {block_id = "block_2", deadlines = {block_0 = 500 : i64, block_1 = 300 : i64}, dependencies = ["block_1"], predecessors = [], prev_comm = "", prev_ent = ""}
    %1 = qoalahost.call @__qoala_wrapper1(%0) : (i32) -> i1
  ^bb3:
    qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = ["block_0", "block_2"], predecessors = [], prev_comm = "", prev_ent = ""}
    %2 = arith.addi %cst, %1 : i1
    qoalahost.return
  }
}
