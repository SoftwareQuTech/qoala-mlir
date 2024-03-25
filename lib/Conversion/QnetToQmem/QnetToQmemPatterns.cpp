#include "Conversion/QnetToQmem/QnetToQmemPatterns.h"

qoala::conversion::QnetToQmemQubitTypeConverter::QnetToQmemQubitTypeConverter(MLIRContext *ctx) {
    // Default conversion for non qnet::QubitType instances
    addConversion([](Type type) { return type; });
    addConversion([ctx](qnet::QubitType type) -> Type {
        return lir::QubitType::get(ctx);
    });
    // In case there will be illegal values (values of types declared illegal) after conversion, then we
    // need to provide a "materialization", i.e. how to cast from one type to the other
    // Here we provide a "cast" to transform from source type (qnet::QubitType) to target type (lir::QubitType)
    addTargetMaterialization([&](
            OpBuilder &builder, Type resultType,
            ValueRange inputs, Location loc) -> Value {
        //The conversion is simply a "builtin::unrealized_cast" operation
        return builder.create<UnrealizedConversionCastOp>(loc, resultType, inputs).getResult(0);
    });
    // Here we provide a "cast" to transform from target type (lir::QubitType) to source type (qnet::QubitType)
    addSourceMaterialization([&](
            OpBuilder &builder, Type resultType,
            ValueRange inputs, Location loc) -> std::optional<Value> {
        //The conversion is simply a "builtin::unrealized_cast" operation
        return builder.create<UnrealizedConversionCastOp>(loc, resultType, inputs).getResult(0);
    });
}
