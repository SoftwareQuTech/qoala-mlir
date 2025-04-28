// RUN: qoala-translate %s --mlir-to-iqoala | FileCheck %s
// CHECK: META START
// CHECK-NEXT: name: test_call_request_routines
// CHECK-NEXT: parameters: Bob_id
// CHECK-NEXT: csockets: 0 -> Bob
// CHECK-NEXT: epr_sockets: 0 -> Bob
// CHECK-NEXT: META END
// CHECK: b[[BLOCK0:.*]] { type = QC }
// CHECK-NEXT: run_request() : __qoala_wrapper0
// CHECK: b[[BLOCK1:.*]] { type = QL }
// CHECK-NEXT: run_subroutine() : __qoala_wrapper1
// CHECK: b[[BLOCK1:.*]] { type = QC }
// CHECK-NEXT: run_request() : __qoala_wrapper2
// CHECK: b[[BLOCK1:.*]] { type = QL }
// CHECK-NEXT: run_subroutine() : __qoala_wrapper3
// CHECK: b[[BLOCK1:.*]] { type = QL }
// CHECK-NEXT: tuple<[[MEAS_0:.*]]; [[MEAS_1:.*]]; [[MEAS_2:.*]]; [[MEAS_3:.*]]; [[MEAS_4:.*]]> = run_subroutine() : __qoala_wrapper4

//CHECK: SUBROUTINE __qoala_wrapper1
// CHECK-NEXT: params: {{[[:space:]]}}
// CHECK-SAME: returns: {{[[:space:]]}}
// CHECK-SAME: uses: [[QUBIT3:.*]]
// CHECK-NEXT: keeps: [[QUBIT3]]
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set [[Q_REGA:.*]] [[QUBIT3]]
// CHECK-NEXT: init [[Q_REGA:.*]]
// CHECK-NEXT: NETQASM_END

// This routine simply uses all the qubits
//CHECK: SUBROUTINE __qoala_wrapper3
// CHECK-NEXT: params: {{[[:space:]]}}
// CHECK-SAME: returns: {{[[:space:]]}}
// CHECK-SAME: uses: [[QUBIT0:.*]], [[QUBIT1:.*]], [[QUBIT2:.*]], [[QUBIT3]], [[QUBIT4:.*]]
// CHECK-NEXT: keeps: [[QUBIT0]], [[QUBIT1]], [[QUBIT2]], [[QUBIT3]], [[QUBIT4]]
// CHECK-NEXT: NETQASM_START
// CHECK-NEXT: set [[Q_REGB0:.*]] [[QUBIT0]]
// CHECK-NEXT: set [[Q_REGB1:.*]] [[QUBIT1]]
// CHECK-NEXT: set [[Q_REGB2:.*]] [[QUBIT2]]
// CHECK-NEXT: set [[Q_REGB3:.*]] [[QUBIT3]]
// CHECK-NEXT: set [[Q_REGB4:.*]] [[QUBIT4]]
// CHECK-NEXT: rot_x [[Q_REGB0]] 0 0
// CHECK-NEXT: rot_y [[Q_REGB1]] 1 0
// CHECK-NEXT: rot_z [[Q_REGB2]] 1 1
// CHECK-NEXT: cnot [[Q_REGB3]] [[Q_REGB4]]
// CHECK-NEXT: NETQASM_END

//CHECK: SUBROUTINE __qoala_wrapper4
// CHECK-NEXT: params: {{[[:space:]]}}
// CHECK-SAME: returns: m0, m1, m2, m3, m4
// CHECK-NEXT: uses: [[QUBIT0]], [[QUBIT1]], [[QUBIT2]], [[QUBIT3]], [[QUBIT4]]
// CHECK-NEXT: keeps: {{[[:space:]]}}
// CHECK-SAME: NETQASM_START
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
// CHECK-NEXT: callback_type: sequential
// CHECK-NEXT: callback: {{[[:space:]]}}
// CHECK-SAME: return_vars: {{[[:space:]]}}
// CHECK-SAME: remote_id: {Bob_id}
// CHECK-NEXT: epr_socket_id: 0
// CHECK-NEXT: num_pairs: 3
// CHECK-NEXT: virt_ids: increment 0
// CHECK-NEXT: timeout: 1000
// CHECK-NEXT: fidelity: 1.000000e+00
// CHECK-NEXT: type: create_keep
// CHECK-NEXT: role: create

//CHECK: REQUEST __qoala_wrapper2
// CHECK-NEXT: callback_type: sequential
// CHECK-NEXT: callback: {{[[:space:]]}}
// CHECK-SAME: return_vars: {{[[:space:]]}}
// CHECK-SAME: remote_id: {Bob_id}
// CHECK-NEXT: epr_socket_id: 0
// CHECK-NEXT: num_pairs: 1
// CHECK-NEXT: virt_ids: all 4
// CHECK-NEXT: timeout: 1000
// CHECK-NEXT: fidelity: 1.000000e+00
// CHECK-NEXT: type: create_keep
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
  netqasm.local_routine @__qoala_wrapper3(%qubit0: i32, %qubit1: i32, %qubit2: i32, %qubit3: i32, %qubit4: i32) -> () {
    netqasm.rot_x %qubit0 (0 : ui32, 0 : ui32)
    netqasm.rot_y %qubit1 (1 : ui32, 0 : ui32)
    netqasm.rot_z %qubit2 (1 : ui32, 1 : ui32)
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
    // Note: there is an implicit "^bb0" not-rendered block declaration here
    // so this "call" operation is the one and only operation of the
    // first block of the main function
    %0, %1, %2 = qoalahost.call @__qoala_wrapper0() : () -> (i32, i32, i32)
    ^bb1:
      %3 = qoalahost.call @__qoala_wrapper1() : () -> i32
    ^bb2:
      %4 = qoalahost.call @__qoala_wrapper2() : () -> i32
    ^bb3:
      qoalahost.call @__qoala_wrapper3(%0, %1, %2, %3, %4) : (i32, i32, i32, i32, i32) -> ()
    ^bb4:
      %m0, %m1, %m2, %m3, %m4 = qoalahost.call @__qoala_wrapper4(%0, %1, %2, %3, %4) : (i32, i32, i32, i32, i32) -> (i1, i1, i1, i1, i1)
    ^bb5:
      qoalahost.return
  }
}
