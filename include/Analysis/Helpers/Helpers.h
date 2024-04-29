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
    bool operationIsNotFromCommonDialects(Operation &);
    std::string getAllowedDialectNames();

    /* Helpers to expose the conversion patterns from QMemToQoalaHost
     * and from QMemtoNetQASM
     * WARNING: The definitions of these functions are in the respective CPP files,
     * so they can be used both in the general MIR to LIR wrapper but also in the
     * passes they belong to.
     */
    void populateQNetToQoalaHostPatterns(RewritePatternSet &, TypeConverter &);
    void populateQNetToNetQASMPatterns(RewritePatternSet &, TypeConverter &);

    class NullTypeConverter : public TypeConverter {
    public:
        explicit NullTypeConverter(MLIRContext *ctx);
    };
}


#endif //QOALA_ANALYSIS_MLIR_HELPERS_H
