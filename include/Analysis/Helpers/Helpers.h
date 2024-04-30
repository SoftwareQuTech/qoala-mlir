#ifndef QOALA_ANALYSIS_MLIR_HELPERS_H
#define QOALA_ANALYSIS_MLIR_HELPERS_H

#include "mlir/IR/Operation.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Transforms/DialectConversion.h"

using namespace mlir;

namespace qoala::helpers {
    /**
     * Determines if the given operations belongs to one of the allowed dialects: 'arith',
     * 'cf', 'memref', 'affine' or 'tensor'.
     * @param operation
     * @return
     */
    bool operationIsNotFromCommonDialects(Operation &operation);
    std::string getAllowedDialectNames();

    /* Helper functions  to expose the conversion patterns from QMemToQoalaHost
     * and QMemtoNetQASM, to use them in the QoalaMIRToQoalaLIR general wrapper pass.
     * WARNING: The definitions of these functions are in the respective CPP files,
     * so they can be used both in the general MIR to LIR wrapper but also in the
     * passes they belong to.
     */

    /**
     * Adds the QNet to QoalaHost conversions patterns to the given rewrite pattern set.
     * It also uses the given type converter.
     * @param context The MLIRContext object.
     * @param patterns The pattern set object to populate.
     * @param typeConverter The type converter object used by the rewriter methods.
     */
    void populateQNetToQoalaHostPatterns(MLIRContext &context, RewritePatternSet &patterns, TypeConverter &typeConverter);

    /**
     * Adds the QNet to NetQASM conversions patterns to the given rewrite pattern set.
     * It also uses the given type converter.
     * @param context The MLIRContext object.
     * @param patterns The pattern set object to populate.
     * @param typeConverter The type converter object used by the rewriter methods.
     */
    void populateQNetToNetQASMPatterns(MLIRContext &context, RewritePatternSet &patterns, TypeConverter &typeConverter);

    /**
     * Simple "null" type converter for dialect conversion passes. This type
     * converter simply returns the same type for any given type.
     */
    class NullTypeConverter : public TypeConverter {
    public:
        explicit NullTypeConverter(MLIRContext *ctx);
    };
}


#endif //QOALA_ANALYSIS_MLIR_HELPERS_H
