#ifndef QOALATRANSLATIONINTERFACE_H
#define QOALATRANSLATIONINTERFACE_H

#include "mlir/IR/DialectInterface.h"
#include "mlir/Support/LogicalResult.h"

namespace qoala::translate {
    class ModuleTranslation;
}

using namespace mlir;
using namespace qoala::translate;

namespace qoala::iqoala {
    class QoalaTranslationDialectInterface
            : public mlir::DialectInterface::Base<QoalaTranslationDialectInterface> {
    public:
        explicit QoalaTranslationDialectInterface(Dialect *dialect) : Base(dialect) {}

        /// Hook for derived dialect interface to provide translation of the
        /// operations to iQoala
        virtual LogicalResult convertOperation(
                Operation *op, ModuleTranslation &moduleTranslation) const {
            // TODO - Check what else we need to pass as arguments...
            //  * An "IR builder"? (factory to create objects that can easily be mapped to iQoala)
            //  * A map between the original op and the iQoala object?
            return failure();
        }
    };

    class QoalaTranslationInterfaces
            : public mlir::DialectInterfaceCollection<QoalaTranslationDialectInterface> {
    public:
        using Base::Base;

        /// Translates the given operation to LLVM IR using the interface implemented
        /// by the op's dialect.
        virtual LogicalResult convertOperation(Operation *op, ModuleTranslation &moduleTranslation) const {
            if (const QoalaTranslationDialectInterface *iface = getInterfaceFor(op))
                return iface->convertOperation(op, moduleTranslation);
            return failure();
        }
    };
} // namespace qoala::iqoala

#endif //QOALATRANSLATIONINTERFACE_H
