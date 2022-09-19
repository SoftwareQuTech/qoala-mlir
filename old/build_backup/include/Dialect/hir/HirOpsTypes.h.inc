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
namespace hir {
  class CValueType;
  class QubitType;

  class CValueType : public ::mlir::Type::TypeBase<CValueType, ::mlir::Type, ::mlir::TypeStorage> {
  public:
    /// Inherit some necessary constructors from 'TypeBase'.
    using Base::Base;
    static constexpr ::llvm::StringLiteral getMnemonic() {
      return ::llvm::StringLiteral("cvalue");
    }
  };

  class QubitType : public ::mlir::Type::TypeBase<QubitType, ::mlir::Type, ::mlir::TypeStorage> {
  public:
    /// Inherit some necessary constructors from 'TypeBase'.
    using Base::Base;
    static constexpr ::llvm::StringLiteral getMnemonic() {
      return ::llvm::StringLiteral("qubit");
    }
  };
} // namespace hir
} // namespace mlir
DECLARE_EXPLICIT_TYPE_ID(::mlir::hir::CValueType)
DECLARE_EXPLICIT_TYPE_ID(::mlir::hir::QubitType)

#endif  // GET_TYPEDEF_CLASSES

