// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s

// CHECK: META_START
// CHECK-NEXT: name: test_call_request_routines
// CHECK-NEXT: parameters: Bob_id
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META_END
// CHECK: b0 { type = QC; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: %0 = run_request() : __qoala_wrapper0
// CHECK: b1 { type = QL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: %1 = run_subroutine() : __qoala_wrapper1
// CHECK: b2 { type = QL; predecessors = []; dependencies = [b0, b1]; prev_comm = ; prev_ent = ; deadlines = [] }:
// CHECK-NEXT: tuple<%2; %3> = run_subroutine() : __qoala_wrapper2
// CHECK: b3 { type = QC; predecessors = []; dependencies = []; prev_comm = ; prev_ent = b0; deadlines = [] }:
// CHECK-NEXT: %4 = run_request() : __qoala_wrapper3

// CHECK: SUBROUTINE __qoala_wrapper1
// CHECK-NEXT: params: {{[[:space:]]}}
// CHECK-SAME: returns: {{[[:space:]]}}
// CHECK-SAME: uses: [[QUBIT_1:.*]]
// CHECK-NEXT: keeps: [[QUBIT_1]]
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set Q0 [[QUBIT_1]]
// CHECK-NEXT: init Q0
// CHECK-NEXT: NETQASM_END

// CHECK: SUBROUTINE __qoala_wrapper2
// CHECK-NEXT: params: {{[[:space:]]}}
// CHECK-SAME: returns: m0, m1
// CHECK-NEXT: uses: [[QUBIT_0:.*]], [[QUBIT_1]]
// CHECK-NEXT: keeps: {{[[:space:]]}}
// CHECK-SAME: NETQASM_START
// CHECK-NEXT: set [[QREG_0:.*]] [[QUBIT_0]]
// CHECK-NEXT: set [[QREG_1:.*]] [[QUBIT_1]]
// CHECK-NEXT: cnot [[QREG_0]] [[QREG_1]]
// CHECK-NEXT: h [[QREG_0]]
// CHECK-NEXT: meas [[QREG_0]] [[MREG_0:.*]]
// CHECK-NEXT: meas [[QREG_1]] [[MREG_1:.*]]
// CHECK-NEXT: store [[MREG_0]] @output[0]
// CHECK-NEXT: store [[MREG_1]] @output[1]
// CHECK-NEXT: NETQASM_END

// CHECK: REQUEST __qoala_wrapper0
// CHECK-NEXT: callback_type: sequential
// CHECK-NEXT: callback: {{[[:space:]]}}
// CHECK-SAME: return_vars: {{[[:space:]]}}
// CHECK-SAME: remote_id: {Bob_id}
// CHECK-NEXT: epr_socket_id: 0
// CHECK-NEXT: num_pairs: 1
// CHECK-NEXT: virt_ids: all [[QUBIT_0]]
// CHECK-NEXT: timeout: 1000
// CHECK-NEXT: fidelity: 1.000000e+00
// CHECK-NEXT: typ: create_keep
// CHECK-NEXT: role: create

// CHECK: REQUEST __qoala_wrapper3
// CHECK-NEXT: callback_type: sequential
// CHECK-NEXT: callback: {{[[:space:]]}}
// CHECK-SAME: return_vars: {{[[:space:]]}}
// CHECK-SAME: remote_id: {Bob_id}
// CHECK-NEXT: epr_socket_id: 0
// CHECK-NEXT: num_pairs: 1
// *IMPORTANT*: This request routine is expected to reuse qubit 0
// CHECK-NEXT: virt_ids: all [[QUBIT_0]]
// CHECK-NEXT: timeout: 1000
// CHECK-NEXT: fidelity: 1.000000e+00
// CHECK-NEXT: typ: create_keep
// CHECK-NEXT: role: create

module {
  qremote.remote @Bob
  netqasm.request_routine @__qoala_wrapper0() -> (i32) {
    %0 = netqasm.qalloc : i32
    netqasm.eprs %0  {remote = @Bob}
    netqasm.return %0 : i32
  }
  netqasm.local_routine @__qoala_wrapper1() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.local_routine @__qoala_wrapper2(%qubit0: i32, %qubit1: i32) -> (i1, i1) {
    netqasm.cnot %qubit0, %qubit1
    netqasm.hadamard %qubit0
    %0 = netqasm.measure %qubit0  : i1
    %1 = netqasm.measure %qubit1  : i1
    netqasm.return %0, %1 : i1, i1
  }
  netqasm.request_routine @__qoala_wrapper3() -> (i32) {
    %0 = netqasm.qalloc : i32
    netqasm.eprs %0  {remote = @Bob}
    netqasm.return %0 : i32
  }
  qoalahost.main_func @test_call_request_routines() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %q0 = qoalahost.call @__qoala_wrapper0() : () -> (i32)
    ^bb1:
      qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
      %q1 = qoalahost.call @__qoala_wrapper1() : () -> i32
    ^bb2:
      qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = ["block_0", "block_1"], predecessors = [], prev_comm = "", prev_ent = ""}
      %m0, %m1 = qoalahost.call @__qoala_wrapper2(%q0, %q1) : (i32, i32) -> (i1, i1)
    ^bb3:
      qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = "block_0"}
      %q2 = qoalahost.call @__qoala_wrapper3() : () -> (i32)
    ^bb4:
      qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
      qoalahost.return
  }
}