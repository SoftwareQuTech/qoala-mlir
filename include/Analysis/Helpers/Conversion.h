#ifndef QOALA_MLIR_CONVERSION_H
#define QOALA_MLIR_CONVERSION_H

#include "mlir/IR/MLIRContext.h"
#include "mlir/Transforms/DialectConversion.h"

namespace qoala::helpers::conversion {
    /* Helper functions to expose the conversion patterns from QMemToQoalaHost
     * and QMemToNetQASM, to use them in the QoalaMIRToQoalaLIR general wrapper pass.
     * WARNING: The definitions of these functions are in the respective CPP files,
     * so they can be used both in the general MIR to LIR wrapper but also in the
     * passes they belong to.
     */

    /**
     * Simple "null" type converter for dialect conversion passes. This type
     * converter simply returns the same type for any given type.
     */
    class NullTypeConverter : public mlir::TypeConverter {
    public:
        explicit NullTypeConverter(mlir::MLIRContext *ctx);
    };
} // namespace qoala::helpers::conversion

#endif // QOALA_MLIR_CONVERSION_H
