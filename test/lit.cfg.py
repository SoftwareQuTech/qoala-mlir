import lit.formats

config.name = "Qoala MLIR"
config.test_format = lit.formats.ShTest(True)
config.suffixes = [".mlir"]

config.llvm_tools_dir = "./llvm/build/bin"
