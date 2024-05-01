#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "Conversion/Helpers/Helpers.h"


namespace qoala::helpers::angle {

    std::string angleConversionFunctionName("__qoala_convert_float_angle");

    bool moduleContainsAngleConversionDeclaration(ModuleOp &module) {
        auto functionDeclaration = module.lookupSymbol<func::FuncOp>(angleConversionFunctionName);
        return functionDeclaration;
    }

    Operation *insertAngleConversionFunctionDeclaration(ModuleOp &module) {
        OpBuilder builder(module.getBodyRegion());
        FunctionType functionType = builder.getFunctionType(
                builder.getF32Type(),
                {builder.getI32Type(), builder.getI32Type()});
        auto funcDeclaration = builder.create<func::FuncOp>(
                module->getLoc(),
                StringRef{angleConversionFunctionName},
                functionType);
        // We set an attribute on the function, so we can recognize it later
        Attribute attr = builder.getStringAttr("true");
        funcDeclaration->setAttr("qoala-builtin", attr);
        // Function declarations must have the private visibility
        funcDeclaration.setVisibility(func::FuncOp::Visibility::Private);
        return funcDeclaration;
    }
}