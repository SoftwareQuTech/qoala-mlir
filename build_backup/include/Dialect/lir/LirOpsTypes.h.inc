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
namespace lir {
  class CValueCType;
  class CValueQType;
  class QubitType;

  class CValueCType : public ::mlir::Type::TypeBase<CValueCType, ::mlir::Type, ::mlir::TypeStorage> {
  public:
    /// Inherit some necessary constructors from 'TypeBase'.
    using Base::Base;
    static constexpr ::llvm::StringLiteral getMnemonic() {
      return ::llvm::StringLiteral("cvalue_c");
    }
  };

  class CValueQType : public ::mlir::Type::TypeBase<CValueQType, ::mlir::Type, ::mlir::TypeStorage> {
  public:
    /// Inherit some necessary constructors from 'TypeBase'.
    using Base::Base;
    static constexpr ::llvm::StringLiteral getMnemonic() {
      return ::llvm::StringLiteral("cvalue_q");
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
} // namespace lir
} // namespace mlir
DECLARE_EXPLICIT_TYPE_ID(::mlir::lir::CValueCType)
DECLARE_EXPLICIT_TYPE_ID(::mlir::lir::CValueQType)
DECLARE_EXPLICIT_TYPE_ID(::mlir::lir::QubitType)

#endif  // GET_TYPEDEF_CLASSES

