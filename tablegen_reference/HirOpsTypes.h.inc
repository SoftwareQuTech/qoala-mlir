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
  class QubitType;

  namespace detail {
    struct QubitTypeStorage;
  } // end namespace detail
  class QubitType : public ::mlir::Type::TypeBase<QubitType, ::mlir::Type,
                                         detail::QubitTypeStorage> {
  public:
    /// Inherit some necessary constructors from 'TypeBase'.
    using Base::Base;

    static QubitType get(::mlir::MLIRContext *context, int64_t size);
    static constexpr ::llvm::StringLiteral getMnemonic() {
      return ::llvm::StringLiteral("qubit");
    }

    static ::mlir::Type parse(::mlir::DialectAsmParser &parser);
    void print(::mlir::DialectAsmPrinter &printer) const;
    int64_t getSize() const;
  };
} // namespace hir
} // namespace mlir
DECLARE_EXPLICIT_TYPE_ID(::mlir::hir::QubitType)

#endif  // GET_TYPEDEF_CLASSES

