// Run with ./bin/interpreter ../programs/llvm/llvm2.mlir -e entry

module {
    llvm.func @entry() -> f32 {
        %var0 = "llvm.mlir.constant"() {value = 3.0: f32} : () -> f32
        llvm.return %var0 : f32
    }
}