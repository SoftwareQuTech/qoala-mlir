/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* TypeDef Definitions                                                        *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifdef GET_TYPEDEF_LIST
#undef GET_TYPEDEF_LIST

::mlir::qlir::CValueType,
::mlir::qlir::QubitType

#endif  // GET_TYPEDEF_LIST

#ifdef GET_TYPEDEF_CLASSES
#undef GET_TYPEDEF_CLASSES


static ::mlir::OptionalParseResult generatedTypeParser(
                                      ::mlir::DialectAsmParser &parser,
                                      ::llvm::StringRef mnemonic,
                                      ::mlir::Type &value) {
  if (mnemonic == ::mlir::qlir::CValueType::getMnemonic()) { 
    value = ::mlir::qlir::CValueType::get(parser.getContext());
    return ::mlir::success(!!value);
  }
  if (mnemonic == ::mlir::qlir::QubitType::getMnemonic()) { 
    value = ::mlir::qlir::QubitType::get(parser.getContext());
    return ::mlir::success(!!value);
  }
  return {};
}


static ::mlir::LogicalResult generatedTypePrinter(
                         ::mlir::Type def, ::mlir::DialectAsmPrinter &printer) {
  return ::llvm::TypeSwitch<::mlir::Type, ::mlir::LogicalResult>(def)
    .Case<::mlir::qlir::CValueType>([&](::mlir::qlir::CValueType t) {
      printer << ::mlir::qlir::CValueType::getMnemonic();
      return ::mlir::success();
    })
    .Case<::mlir::qlir::QubitType>([&](::mlir::qlir::QubitType t) {
      printer << ::mlir::qlir::QubitType::getMnemonic();
      return ::mlir::success();
    })
    .Default([](::mlir::Type) { return ::mlir::failure(); });
}

namespace mlir {
namespace qlir {
} // namespace qlir
} // namespace mlir
DEFINE_EXPLICIT_TYPE_ID(::mlir::qlir::CValueType)
namespace mlir {
namespace qlir {
} // namespace qlir
} // namespace mlir
DEFINE_EXPLICIT_TYPE_ID(::mlir::qlir::QubitType)

#endif  // GET_TYPEDEF_CLASSES

