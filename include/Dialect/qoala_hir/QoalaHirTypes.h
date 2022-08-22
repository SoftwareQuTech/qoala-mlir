#ifndef QOALA_HIR_TYPES_H
#define QOALA_HIR_TYPES_H

#include "mlir/IR/TypeSupport.h"
#include "mlir/IR/Types.h"
#include "mlir/IR/BuiltinTypes.h"

#include "Dialect/qoala_hir/QoalaHirTypes.h.inc"

struct IntStructTypeStorage : public mlir::TypeStorage
{
    using KeyTy = llvm::ArrayRef<mlir::IntegerType>;

    IntStructTypeStorage(llvm::ArrayRef<mlir::IntegerType> elementTypes)
        : elementTypes(elementTypes) {}

    bool operator==(const KeyTy &key) const { return key == elementTypes; }

    llvm::ArrayRef<mlir::IntegerType> elementTypes;

    static IntStructTypeStorage *construct(mlir::TypeStorageAllocator &allocator,
                                           const KeyTy &key)
    {
        llvm::ArrayRef<mlir::IntegerType> elementTypes = allocator.copyInto(key);

        return new (allocator.allocate<IntStructTypeStorage>())
            IntStructTypeStorage(elementTypes);
    }
};

class IntStructType : public mlir::Type::TypeBase<IntStructType, mlir::Type,
                                                  IntStructTypeStorage>
{
public:
    using Base::Base;

    static IntStructType get(llvm::ArrayRef<mlir::IntegerType> elementTypes)
    {
        assert(!elementTypes.empty());

        mlir::MLIRContext *ctx = elementTypes.front().getContext();
        return Base::get(ctx, elementTypes);
    }

    llvm::ArrayRef<mlir::IntegerType> getElementTypes()
    {
        return getImpl()->elementTypes;
    }

    size_t getNumElementTypes() { return getElementTypes().size(); }
};

#endif // QOALA_HIR_TYPES_H