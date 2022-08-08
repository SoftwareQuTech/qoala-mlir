/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* TypeDef Definitions                                                        *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifdef GET_TYPEDEF_LIST
#undef GET_TYPEDEF_LIST

::mlir::hir::CValueType,
::mlir::hir::QubitType

#endif  // GET_TYPEDEF_LIST

#ifdef GET_TYPEDEF_CLASSES
#undef GET_TYPEDEF_CLASSES


static ::mlir::OptionalParseResult generatedTypeParser(
                                      ::mlir::DialectAsmParser &parser,
                                      ::llvm::StringRef mnemonic,
                                      ::mlir::Type &value) {
  if (mnemonic == ::mlir::hir::CValueType::getMnemonic()) { 
    value = ::mlir::hir::CValueType::get(parser.getContext());
    return ::mlir::success(!!value);
  }
  if (mnemonic == ::mlir::hir::QubitType::getMnemonic()) { 
    value = ::mlir::hir::QubitType::get(parser.getContext());
    return ::mlir::success(!!value);
  }
  return {};
}


static ::mlir::LogicalResult generatedTypePrinter(
                         ::mlir::Type def, ::mlir::DialectAsmPrinter &printer) {
  return ::llvm::TypeSwitch<::mlir::Type, ::mlir::LogicalResult>(def)
    .Case<::mlir::hir::CValueType>([&](::mlir::hir::CValueType t) {
      printer << ::mlir::hir::CValueType::getMnemonic();
      return ::mlir::success();
    })
    .Case<::mlir::hir::QubitType>([&](::mlir::hir::QubitType t) {
      printer << ::mlir::hir::QubitType::getMnemonic();
      return ::mlir::success();
    })
    .Default([](::mlir::Type) { return ::mlir::failure(); });
}

namespace mlir {
namespace hir {
} // namespace hir
} // namespace mlir
DEFINE_EXPLICIT_TYPE_ID(::mlir::hir::CValueType)
namespace mlir {
namespace hir {
} // namespace hir
} // namespace mlir
DEFINE_EXPLICIT_TYPE_ID(::mlir::hir::QubitType)

#endif  // GET_TYPEDEF_CLASSES

