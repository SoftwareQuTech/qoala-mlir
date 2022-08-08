/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* TypeDef Definitions                                                        *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifdef GET_TYPEDEF_LIST
#undef GET_TYPEDEF_LIST

::mlir::clir::CMsgType,
::mlir::clir::CLirValueType

#endif  // GET_TYPEDEF_LIST

#ifdef GET_TYPEDEF_CLASSES
#undef GET_TYPEDEF_CLASSES


static ::mlir::OptionalParseResult generatedTypeParser(
                                      ::mlir::DialectAsmParser &parser,
                                      ::llvm::StringRef mnemonic,
                                      ::mlir::Type &value) {
  if (mnemonic == ::mlir::clir::CMsgType::getMnemonic()) { 
    value = ::mlir::clir::CMsgType::get(parser.getContext());
    return ::mlir::success(!!value);
  }
  if (mnemonic == ::mlir::clir::CLirValueType::getMnemonic()) { 
    value = ::mlir::clir::CLirValueType::get(parser.getContext());
    return ::mlir::success(!!value);
  }
  return {};
}


static ::mlir::LogicalResult generatedTypePrinter(
                         ::mlir::Type def, ::mlir::DialectAsmPrinter &printer) {
  return ::llvm::TypeSwitch<::mlir::Type, ::mlir::LogicalResult>(def)
    .Case<::mlir::clir::CMsgType>([&](::mlir::clir::CMsgType t) {
      printer << ::mlir::clir::CMsgType::getMnemonic();
      return ::mlir::success();
    })
    .Case<::mlir::clir::CLirValueType>([&](::mlir::clir::CLirValueType t) {
      printer << ::mlir::clir::CLirValueType::getMnemonic();
      return ::mlir::success();
    })
    .Default([](::mlir::Type) { return ::mlir::failure(); });
}

namespace mlir {
namespace clir {
} // namespace clir
} // namespace mlir
DEFINE_EXPLICIT_TYPE_ID(::mlir::clir::CMsgType)
namespace mlir {
namespace clir {
} // namespace clir
} // namespace mlir
DEFINE_EXPLICIT_TYPE_ID(::mlir::clir::CLirValueType)

#endif  // GET_TYPEDEF_CLASSES

