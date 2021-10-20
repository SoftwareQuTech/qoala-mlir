// RUN: hir-opt %s

%0 = "hir.qalloc"() : !hir.qubit<>
"hir.use_qubit"(%0) : (!hir.qubit<>) -> ()

// CHECK: Module:
// CHECK-NEXT: module {
// CHECK-NEXT: }
