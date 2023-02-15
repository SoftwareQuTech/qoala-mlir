// Run with ./bin/interpreter ../programs/llvm/llvm3.mlir -e entry

module {
    func.func @entry() -> f32 {
        %var0 = "llvm.mlir.constant"() {value = 3.0: f32} : () -> f32
        func.return %var0 : f32
    }
}