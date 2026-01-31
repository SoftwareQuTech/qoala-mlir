// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META_START
// CHECK-NEXT: name: test_call_request_routines
// CHECK-NEXT: parameters: Bob_id
// CHECK-NEXT: csockets:
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META_END
// CHECK: ^b[[BLOCK0:.*]] { type = CL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: %[[VAL_0:.+]] = assign_cval() : 0
// CHECK-NEXT: %[[VAL_1:.+]] = assign_cval() : 1
// CHECK: ^b[[BLOCK1:.*]] { type = QC; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: run_request() : __qoala_wrapper0
// CHECK: ^b[[BLOCK2:.*]] { type = QL; predecessors = []; dependencies = []; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: run_subroutine() : __qoala_wrapper1
// CHECK: ^b[[BLOCK3:.*]] { type = QC; predecessors = []; dependencies = []; prev_comm = ; prev_ent = b0; deadlines = [] }
// CHECK-NEXT: run_request() : __qoala_wrapper2
// CHECK: ^b[[BLOCK4:.*]] { type = QL; predecessors = []; dependencies = [b0, b1, b2, b3]; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: run_subroutine(tuple<%[[VAL_0]]; %[[VAL_1]]>) : __qoala_wrapper3
// CHECK: ^b[[BLOCK5:.*]] { type = QL; predecessors = []; dependencies = [b1, b2, b3]; prev_comm = ; prev_ent = ; deadlines = [] }
// CHECK-NEXT: tuple<[[MEAS_0:.*]]; [[MEAS_1:.*]]; [[MEAS_2:.*]]; [[MEAS_3:.*]]; [[MEAS_4:.*]]> = run_subroutine() : __qoala_wrapper4

//CHECK: SUBROUTINE __qoala_wrapper1
// CHECK-NEXT: params: {{[[:space:]]}}
// CHECK-SAME: returns: {{[[:space:]]}}
// CHECK-SAME: uses: [[QUBIT3:.*]]
// CHECK-NEXT: keeps: [[QUBIT3]]
// CHECK-NEXT: request:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set [[Q_REGA:.*]] [[QUBIT3]]
// CHECK-NEXT: init [[Q_REGA:.*]]
// CHECK-NEXT: NETQASM_END

// This routine simply uses all the qubits
//CHECK: SUBROUTINE __qoala_wrapper3
// CHECK-NEXT: params: p5, p6
// CHECK-NEXT: returns: {{[[:space:]]}}
// CHECK-SAME: uses: [[QUBIT0:.*]], [[QUBIT1:.*]], [[QUBIT2:.*]], [[QUBIT3]], [[QUBIT4:.*]]
// CHECK-NEXT: keeps: [[QUBIT0]], [[QUBIT1]], [[QUBIT2]], [[QUBIT3]], [[QUBIT4]]
// CHECK-NEXT: request:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set [[Q_REGB0:.*]] [[QUBIT0]]
// CHECK-NEXT: set [[Q_REGB1:.*]] [[QUBIT1]]
// CHECK-NEXT: set [[Q_REGB2:.*]] [[QUBIT2]]
// CHECK-NEXT: set [[Q_REGB3:.*]] [[QUBIT3]]
// CHECK-NEXT: set [[Q_REGB4:.*]] [[QUBIT4]]
// CHECK-NEXT: load [[VAL0:.*]] @input[5]
// CHECK-NEXT: load [[VAL1:.*]] @input[6]
// CHECK-NEXT: rot_x [[Q_REGB0]] [[VAL0]] [[VAL0]]
// CHECK-NEXT: rot_y [[Q_REGB1]] [[VAL1]] [[VAL0]]
// CHECK-NEXT: rot_z [[Q_REGB2]] [[VAL1]] [[VAL1]]
// CHECK-NEXT: cnot [[Q_REGB3]] [[Q_REGB4]]
// CHECK-NEXT: NETQASM_END

//CHECK: SUBROUTINE __qoala_wrapper4
// CHECK-NEXT: params: {{[[:space:]]}}
// CHECK-SAME: returns: m0, m1, m2, m3, m4
// CHECK-NEXT: uses: [[QUBIT0]], [[QUBIT1]], [[QUBIT2]], [[QUBIT3]], [[QUBIT4]]
// CHECK-NEXT: keeps: {{[[:space:]]}}
// CHECK-SAME: request:
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set [[Q_REG0:.*]] [[QUBIT0]]
// CHECK-NEXT: set [[Q_REG1:.*]] [[QUBIT1]]
// CHECK-NEXT: set [[Q_REG2:.*]] [[QUBIT2]]
// CHECK-NEXT: set [[Q_REG3:.*]] [[QUBIT3]]
// CHECK-NEXT: set [[Q_REG4:.*]] [[QUBIT4]]
// CHECK-NEXT: meas [[Q_REG0]] [[M_REG0:.*]]
// CHECK-NEXT: meas [[Q_REG1]] [[M_REG1:.*]]
// CHECK-NEXT: meas [[Q_REG2]] [[M_REG2:.*]]
// CHECK-NEXT: meas [[Q_REG3]] [[M_REG3:.*]]
// CHECK-NEXT: meas [[Q_REG4]] [[M_REG4:.*]]
// CHECK-NEXT: store [[M_REG0]] @output[0]
// CHECK-NEXT: store [[M_REG1]] @output[1]
// CHECK-NEXT: store [[M_REG2]] @output[2]
// CHECK-NEXT: store [[M_REG3]] @output[3]
// CHECK-NEXT: store [[M_REG4]] @output[4]
// CHECK-NEXT: NETQASM_END

//CHECK: REQUEST __qoala_wrapper0
// CHECK-NEXT: callback_type: WAIT_ALL
// CHECK-NEXT: callback: {{[[:space:]]}}
// CHECK-SAME: return_vars: {{[[:space:]]}}
// CHECK-SAME: remote_id: {Bob_id}
// CHECK-NEXT: epr_socket_id: 0
// CHECK-NEXT: num_pairs: 3
// CHECK-NEXT: virt_ids: increment 0
// CHECK-NEXT: timeout: 1000
// CHECK-NEXT: fidelity: 1.000000e+00
// CHECK-NEXT: typ: create_keep
// CHECK-NEXT: role: create

//CHECK: REQUEST __qoala_wrapper2
// CHECK-NEXT: callback_type: WAIT_ALL
// CHECK-NEXT: callback: {{[[:space:]]}}
// CHECK-SAME: return_vars: {{[[:space:]]}}
// CHECK-SAME: remote_id: {Bob_id}
// CHECK-NEXT: epr_socket_id: 0
// CHECK-NEXT: num_pairs: 1
// CHECK-NEXT: virt_ids: all 4
// CHECK-NEXT: timeout: 1000
// CHECK-NEXT: fidelity: 1.000000e+00
// CHECK-NEXT: typ: create_keep
// CHECK-NEXT: role: create

module {
  qremote.remote @Bob
  netqasm.local_routine private @__qoala_convert_float_angle(f32) -> (i32, i32)
  netqasm.request_routine @__qoala_wrapper0() -> (i32, i32, i32) {
    %0 = netqasm.qalloc  : i32
    %1 = netqasm.qalloc  : i32
    %2 = netqasm.qalloc  : i32
    netqasm.eprs %0  {remote = @Bob}
    netqasm.eprs %1  {remote = @Bob}
    netqasm.eprs %2  {remote = @Bob}
    netqasm.return %0, %1, %2 : i32, i32, i32
  }
  netqasm.local_routine @__qoala_wrapper1() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.init %0
    netqasm.return %0 : i32
  }
  netqasm.request_routine @__qoala_wrapper2() -> i32 {
    %0 = netqasm.qalloc  : i32
    netqasm.eprs %0  {remote = @Bob}
    netqasm.return %0 : i32
  }
  netqasm.local_routine @__qoala_wrapper3(%qubit0: i32, %qubit1: i32, %qubit2: i32, %qubit3: i32, %qubit4: i32, %c0_arg: i32, %c1_arg: i32) -> () {
    netqasm.rot_x %qubit0, %c0_arg, %c0_arg
    netqasm.rot_y %qubit1, %c1_arg, %c0_arg
    netqasm.rot_z %qubit2, %c1_arg, %c1_arg
    netqasm.cnot %qubit3, %qubit4
    netqasm.return
  }
  netqasm.local_routine @__qoala_wrapper4(%qubit0: i32, %qubit1: i32, %qubit2: i32, %qubit3: i32, %qubit4: i32) -> (i1, i1, i1, i1, i1) {
    %0 = netqasm.measure %qubit0  : i1
    %1 = netqasm.measure %qubit1  : i1
    %2 = netqasm.measure %qubit2  : i1
    %3 = netqasm.measure %qubit3  : i1
    %4 = netqasm.measure %qubit4  : i1
    netqasm.return %0, %1, %2, %3, %4 : i1, i1, i1, i1, i1
  }
  qoalahost.main_func @test_call_request_routines() {
    qoalahost.blk_meta  {block_id = "block_0", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
    %c0_i32 = arith.constant 0 : i32
    %c1_i32 = arith.constant 1 : i32
    qoalahost.nop_term
    ^bb0:
      qoalahost.blk_meta  {block_id = "block_1", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
      %0, %1, %2 = qoalahost.call @__qoala_wrapper0() : () -> (i32, i32, i32)
    ^bb1:
      qoalahost.blk_meta  {block_id = "block_2", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
      %3 = qoalahost.call @__qoala_wrapper1() : () -> i32
    ^bb2:
      qoalahost.blk_meta  {block_id = "block_3", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = "block_0"}
      %4 = qoalahost.call @__qoala_wrapper2() : () -> i32
    ^bb3:
      qoalahost.blk_meta  {block_id = "block_4", deadlines = {}, dependencies = ["block_0", "block_1", "block_2", "block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
      qoalahost.call @__qoala_wrapper3(%0, %1, %2, %3, %4, %c0_i32, %c1_i32) : (i32, i32, i32, i32, i32, i32, i32) -> ()
    ^bb4:
      qoalahost.blk_meta  {block_id = "block_5", deadlines = {}, dependencies = ["block_1", "block_2", "block_3"], predecessors = [], prev_comm = "", prev_ent = ""}
      %m0, %m1, %m2, %m3, %m4 = qoalahost.call @__qoala_wrapper4(%0, %1, %2, %3, %4) : (i32, i32, i32, i32, i32) -> (i1, i1, i1, i1, i1)
    ^bb5:
      qoalahost.blk_meta  {block_id = "block_6", deadlines = {}, dependencies = [], predecessors = [], prev_comm = "", prev_ent = ""}
      qoalahost.return
  }
}
