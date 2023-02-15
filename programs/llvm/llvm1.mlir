// Run with ./bin/interpreter ../programs/llvm/llvm1.mlir -e entry -entry-point-result=void

module {
    llvm.func @entry() {
        llvm.return
    }
}