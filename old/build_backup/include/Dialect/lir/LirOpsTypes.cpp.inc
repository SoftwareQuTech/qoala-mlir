/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* TypeDef Definitions                                                        *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifdef GET_TYPEDEF_LIST
#undef GET_TYPEDEF_LIST

::mlir::lir::CValueCType,
::mlir::lir::CValueQType,
::mlir::lir::QubitType

#endif  // GET_TYPEDEF_LIST

#ifdef GET_TYPEDEF_CLASSES
#undef GET_TYPEDEF_CLASSES


static ::mlir::OptionalParseResult generatedTypeParser(
                                      ::mlir::DialectAsmParser &parser,
                                      ::llvm::StringRef mnemonic,
                                      ::mlir::Type &value) {
  if (mnemonic == ::mlir::lir::CValueCType::getMnemonic()) { 
    value = ::mlir::lir::CValueCType::get(parser.getContext());
    return ::mlir::success(!!value);
  }
  if (mnemonic == ::mlir::lir::CValueQType::getMnemonic()) { 
    value = ::mlir::lir::CValueQType::get(parser.getContext());
    return ::mlir::success(!!value);
  }
  if (mnemonic == ::mlir::lir::QubitType::getMnemonic()) { 
    value = ::mlir::lir::QubitType::get(parser.getContext());
    return ::mlir::success(!!value);
  }
  return {};
}


static ::mlir::LogicalResult generatedTypePrinter(
                         ::mlir::Type def, ::mlir::DialectAsmPrinter &printer) {
  return ::llvm::TypeSwitch<::mlir::Type, ::mlir::LogicalResult>(def)
    .Case<::mlir::lir::CValueCType>([&](::mlir::lir::CValueCType t) {
      printer << ::mlir::lir::CValueCType::getMnemonic();
      return ::mlir::success();
    })
    .Case<::mlir::lir::CValueQType>([&](::mlir::lir::CValueQType t) {
      printer << ::mlir::lir::CValueQType::getMnemonic();
      return ::mlir::success();
    })
    .Case<::mlir::lir::QubitType>([&](::mlir::lir::QubitType t) {
      printer << ::mlir::lir::QubitType::getMnemonic();
      return ::mlir::success();
    })
    .Default([](::mlir::Type) { return ::mlir::failure(); });
}

namespace mlir {
namespace lir {
} // namespace lir
} // namespace mlir
DEFINE_EXPLICIT_TYPE_ID(::mlir::lir::CValueCType)
namespace mlir {
namespace lir {
} // namespace lir
} // namespace mlir
DEFINE_EXPLICIT_TYPE_ID(::mlir::lir::CValueQType)
namespace mlir {
namespace lir {
} // namespace lir
} // namespace mlir
DEFINE_EXPLICIT_TYPE_ID(::mlir::lir::QubitType)

#endif  // GET_TYPEDEF_CLASSES

