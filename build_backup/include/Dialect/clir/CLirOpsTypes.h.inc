/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* TypeDef Declarations                                                       *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifdef GET_TYPEDEF_CLASSES
#undef GET_TYPEDEF_CLASSES


namespace mlir {
class DialectAsmParser;
class DialectAsmPrinter;
} // namespace mlir
namespace mlir {
namespace clir {
  class CMsgType;
  class CLirValueType;

  class CMsgType : public ::mlir::Type::TypeBase<CMsgType, ::mlir::Type, ::mlir::TypeStorage> {
  public:
    /// Inherit some necessary constructors from 'TypeBase'.
    using Base::Base;
    static constexpr ::llvm::StringLiteral getMnemonic() {
      return ::llvm::StringLiteral("cmsg");
    }
  };

  class CLirValueType : public ::mlir::Type::TypeBase<CLirValueType, ::mlir::Type, ::mlir::TypeStorage> {
  public:
    /// Inherit some necessary constructors from 'TypeBase'.
    using Base::Base;
    static constexpr ::llvm::StringLiteral getMnemonic() {
      return ::llvm::StringLiteral("value");
    }
  };
} // namespace clir
} // namespace mlir
DECLARE_EXPLICIT_TYPE_ID(::mlir::clir::CMsgType)
DECLARE_EXPLICIT_TYPE_ID(::mlir::clir::CLirValueType)

#endif  // GET_TYPEDEF_CLASSES

