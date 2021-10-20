/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* TypeDef Definitions                                                        *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifdef GET_TYPEDEF_LIST
#undef GET_TYPEDEF_LIST

::mlir::hir::QubitType

#endif  // GET_TYPEDEF_LIST

#ifdef GET_TYPEDEF_CLASSES
#undef GET_TYPEDEF_CLASSES


static ::mlir::OptionalParseResult generatedTypeParser(
                                      ::mlir::DialectAsmParser &parser,
                                      ::llvm::StringRef mnemonic,
                                      ::mlir::Type &value) {
  if (mnemonic == ::mlir::hir::QubitType::getMnemonic()) { 
    value = ::mlir::hir::QubitType::parse(parser);
    return ::mlir::success(!!value);
  }
  return {};
}


static ::mlir::LogicalResult generatedTypePrinter(
                         ::mlir::Type def, ::mlir::DialectAsmPrinter &printer) {
  return ::llvm::TypeSwitch<::mlir::Type, ::mlir::LogicalResult>(def)
    .Case<::mlir::hir::QubitType>([&](::mlir::hir::QubitType t) {
      t.print(printer);
      return ::mlir::success();
    })
    .Default([](::mlir::Type) { return ::mlir::failure(); });
}

namespace mlir {
namespace hir {

namespace detail {
  struct QubitTypeStorage : public ::mlir::TypeStorage {
    QubitTypeStorage (int64_t size)
      : size(size) { }

    /// The hash key is a tuple of the parameter types.
    using KeyTy = std::tuple<int64_t>;
  bool operator==(const KeyTy &tblgenKey) const {
    if (!(size == std::get<0>(tblgenKey)))
      return false;
    return true;
  }
  static ::llvm::hash_code hashKey(const KeyTy &tblgenKey) {
      return ::llvm::hash_combine(std::get<0>(tblgenKey));
    }

    /// Define a construction method for creating a new instance of this
    /// storage.
    static QubitTypeStorage *construct(::mlir::TypeStorageAllocator &allocator,
                          const KeyTy &tblgenKey) {
      auto size = std::get<0>(tblgenKey);

      return new (allocator.allocate<QubitTypeStorage>())
          QubitTypeStorage(size);
    }
      int64_t size;
  };
} // namespace detail
QubitType QubitType::get(::mlir::MLIRContext *context, int64_t size) {
  return Base::get(context, size);
}
int64_t QubitType::getSize() const { return getImpl()->size; }
} // namespace hir
} // namespace mlir
DEFINE_EXPLICIT_TYPE_ID(::mlir::hir::QubitType)

#endif  // GET_TYPEDEF_CLASSES

