// RUN: qoala-opt %s --lower-qoala-mir-to-lir=max-ops-per-group=1 --qoala-opt-group-ent-reqs | FileCheck %s

module {
    qmem.remote @Bob

    qmem.func @test_group_ent() {
        %0 = qmem.qalloc : i32
        qmem.init %0

        %1 = qmem.qalloc : i32
        qmem.eprs %1 {remote = @Bob}

        qmem.hadamard %0

        %2= qmem.qalloc : i32
        qmem.eprs %2 {remote = @Bob}

        qmem.return
    }
}
