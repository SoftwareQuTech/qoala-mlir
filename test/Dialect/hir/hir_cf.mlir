// RUN: hir-opt %s | FileCheck %s

module {
    %i0 = arith.constant 2 : i32
    %i1 = arith.constant 3 : i32
    %f0 = arith.constant 2.0 : f32
    %f1 = arith.constant 3.0 : f32

// CHECK: cmpi eq
    %cmpi_eq = arith.cmpi eq, %i0, %i1 : i32
// CHECK: cmpf oeq

    %cmpf_oeq = arith.cmpf oeq, %f0, %f1 : f32
// CHECK: cmpi slt
    %cmpi_slt = arith.cmpi slt, %i0, %i1 : i32
// CHECK: cmpi sle
    %cmpi_sle = arith.cmpi sle, %i0, %i1 : i32
// CHECK: cmpf olt
    %cmpi_olt = arith.cmpf olt, %f0, %f1 : f32
// CHECK: cmpf ole
    %cmpi_ole = arith.cmpf ole, %f0, %f1 : f32

// CHECK: cmpi sgt
    %cmpi_sgt = arith.cmpi sgt, %i0, %i1 : i32
// CHECK: cmpi sge
    %cmpi_sge = arith.cmpi sge, %i0, %i1 : i32
// CHECK: cmpf ogt
    %cmpi_ogt = arith.cmpf ogt, %f0, %f1 : f32
// CHECK: cmpf oge
    %cmpi_oge = arith.cmpf oge, %f0, %f1 : f32


// CHECK: if
// CHECK: else
    scf.if %cmpi_eq {
    } else {
    }

// CHECK: if
// CHECK: yield
// CHECK: else
// CHECK: yield
    %x = scf.if %cmpi_eq -> i32 {
        scf.yield %i0 : i32
    } else {
        scf.yield %i1 : i32
    }

// CHECK: while
// CHECK: condition
// CHECK: do
// CHECK: yield
    scf.while : () -> () {
        %condition = arith.cmpi eq, %i0, %i1 : i32
        scf.condition(%condition)
    } do {
        scf.yield
    }

// CHECK: execute
// CHECK: yield
    %token, %res_future = async.execute -> !async.value<i32> {
        %res = arith.constant 7: i32
        async.yield %res : i32
    }
// CHECK: await
// CHECK: await
    async.await %token : !async.token
    %result = async.await %res_future : !async.value<i32>

}