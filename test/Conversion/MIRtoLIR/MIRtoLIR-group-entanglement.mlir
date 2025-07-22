// RUN: qoala-opt %s --lower-qoala-mir-to-lir=max-ops-per-group=1 --qoala-opt-group-ent-reqs | FileCheck %s

module {
    qmem.remote @Bob
    qmem.remote @Charlie

    qmem.func @test_group_ent() {
        %0 = qmem.qalloc : i32
        qmem.eprs %0 {remote = @Bob}

        %1 = qmem.qalloc : i32
        qmem.eprs %1 {remote = @Bob}

        %2 = qmem.qalloc : i32
        qmem.eprs %2 {remote = @Charlie}

        %3 = qmem.qalloc : i32
        %4 = qmem.eprs_measure %3 {remote = @Charlie} : i1

        %5 = qmem.qalloc : i32
        %6 = qmem.eprs_measure %5 {remote = @Charlie} : i1

        %7 = qmem.qalloc : i32
        %8 = qmem.eprs_measure %7 {remote = @Bob} : i1

        qmem.hadamard %0
        qmem.hadamard %0
        qmem.hadamard %0

        qmem.return
    }
}
