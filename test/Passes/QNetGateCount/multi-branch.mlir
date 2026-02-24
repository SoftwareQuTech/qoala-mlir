// RUN: qoala-opt %s --qnet-show-analysis-gate-count | FileCheck %s
// CHECK:  [Gate Count]:
// CHECK:  - One-qubit gates: 6
// CHECK:  - Two-qubit gates: 0
// CHECK:  - Total gates: 6
// CHECK:  Detailed gate count:
// CHECK:  - One-qubit gates:
// CHECK:    * qubit[0]: 6
// CHECK:  - Two-qubit gates:
// CHECK:    * qubit[0]: 0
// CHECK:  -  All gates:
// CHECK:    * qubit[0]: 6

module {
  qnet.remote @Alice
  qnet.func @teleport_bob() {
    %0 = qnet.eprs  {remote = @Alice} : !qnet.qubit
    %1 = qnet.recv_int  {remote = @Alice} : i32
    %2 = qnet.recv_int  {remote = @Alice} : i32
    %c1_i32 = arith.constant 1 : i32
    %3 = arith.cmpi eq, %1, %c1_i32 : i32
    %4 = scf.if %3 -> (!qnet.qubit) {
      %8 = qnet.z %0 : !qnet.qubit
      %9 = qnet.x %8 : !qnet.qubit
      scf.yield %9 : !qnet.qubit
    } else {
      %8 = qnet.x %0 : !qnet.qubit
      %9 = qnet.z %8 : !qnet.qubit
      %10 = qnet.y %9 : !qnet.qubit
      %11 = qnet.x %10 : !qnet.qubit
      scf.yield %11 : !qnet.qubit
    }
    %c1_i32_0 = arith.constant 1 : i32
    %5 = arith.cmpi eq, %2, %c1_i32_0 : i32
    %6 = scf.if %5 -> (!qnet.qubit) {
      %8 = qnet.x %4 : !qnet.qubit
      %9 = qnet.z %8 : !qnet.qubit
      scf.yield %9 : !qnet.qubit
    } else {
      %8 = qnet.x %4 : !qnet.qubit
      scf.yield %8 : !qnet.qubit
    }
    %7 = qnet.measure %6 : i1
    qnet.return %7 : i1
  }
}
