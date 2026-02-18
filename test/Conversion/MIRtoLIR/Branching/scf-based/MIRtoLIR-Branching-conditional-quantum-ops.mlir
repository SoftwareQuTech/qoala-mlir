// RUN: qoala-opt %s --lower-qoala-mir-to-lir | FileCheck %s

// Verifies that quantum operations inside scf.if branch arms are NOT merged
// with unconditional operations at the merge point.  Each conditional op must
// end up in its own wrapper routine, called from within its branch arm block.
//
// Without the fix, qmem.z, qmem.x, rot_z, hadamard, and measure would all be
// grouped into a single __qoala_wrapper and called at the merge point, losing
// the conditionality of z and x.

// CHECK: module
module {
  // CHECK: qremote.remote @[[REMOTE:.*]]
  qmem.remote @client
  qmem.func @conditional_quantum_ops() {
    // The EPR request gets its own request_routine.
    // CHECK: netqasm.request_routine @[[EPR_WRAPPER:[^ ]+]]() -> i32
    // CHECK-NEXT: %{{.*}} = netqasm.qalloc
    // CHECK-NEXT: netqasm.eprs

    // qmem.z must be in its own local_routine (no rot_z/hadamard/measure here).
    // CHECK: netqasm.local_routine @[[Z_WRAPPER:[^ ]+]](%{{.*}}: i32) {
    // CHECK-NEXT: netqasm.z
    // CHECK-NEXT: netqasm.return

    // qmem.x must be in its own local_routine.
    // CHECK: netqasm.local_routine @[[X_WRAPPER:[^ ]+]](%{{.*}}: i32) {
    // CHECK-NEXT: netqasm.x
    // CHECK-NEXT: netqasm.return

    // The unconditional rot_z + hadamard + measure are grouped at the merge point.
    // CHECK: netqasm.local_routine @[[MERGE_WRAPPER:[^ ]+]](%{{.*}}: i32, %{{.*}}: i32, %{{.*}}: i32) -> i1
    // CHECK: netqasm.rot_z
    // CHECK: netqasm.hadamard
    // CHECK: netqasm.measure

    %0 = qmem.qalloc : i32
    qmem.eprs %0 {remote = @client}
    %1 = qmem.recv_int {remote = @client} : i32
    %2 = qmem.recv_int {remote = @client} : i32
    %c1_i32 = arith.constant 1 : i32
    %3 = arith.cmpi eq, %1, %c1_i32 : i32
    scf.if %3 {
      qmem.z %0
    }
    %c1_i32_0 = arith.constant 1 : i32
    %4 = arith.cmpi eq, %2, %c1_i32_0 : i32
    scf.if %4 {
      qmem.x %0
    }
    %c8_i32 = arith.constant 8 : i32
    %c2_i32 = arith.constant 2 : i32
    qmem.rot_z_int %0, %c8_i32, %c2_i32
    qmem.hadamard %0
    %5 = qmem.measure %0 : i1
    %6 = arith.extui %5 : i1 to i32
    qmem.send_int %6 {remote = @client} : i32
    qmem.return
  }
}

// The call to the z-wrapper must appear inside the first branch arm (^bb7),
// NOT at the merge block (^bb12).  The call to x-wrapper must appear in the
// second branch arm (^bb10).  The merge wrapper is called at ^bb12.
//
// CHECK: cf.cond_br %{{.*}}, ^bb7, ^bb9
// CHECK-NEXT: ^bb7:
// CHECK: qoalahost.call @[[Z_WRAPPER]]
// CHECK: cf.cond_br %{{.*}}, ^bb10, ^bb12
// CHECK-NEXT: ^bb10:
// CHECK: qoalahost.call @[[X_WRAPPER]]
// CHECK: ^bb12:
// CHECK: qoalahost.call @[[MERGE_WRAPPER]]
