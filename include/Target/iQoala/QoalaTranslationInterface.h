#ifndef QOALATRANSLATIONINTERFACE_H
#define QOALATRANSLATIONINTERFACE_H

#include "mlir/IR/DialectInterface.h"
#include "mlir/Support/LogicalResult.h"

// Forward declaration to avoid circular includes
namespace qoala::translate {
    class ModuleTranslation;
}

namespace qoala::iqoala {
    class QoalaTranslationDialectInterface
            : public mlir::DialectInterface::Base<QoalaTranslationDialectInterface> {
    public:
        explicit QoalaTranslationDialectInterface(mlir::Dialect *dialect) : Base(dialect) {}

        /// Hook for derived dialect interface to provide translation of the
        /// operations to iQoala
        virtual mlir::LogicalResult convertOperation(
                mlir::Operation *op, translate::ModuleTranslation &moduleTranslation) const {
            // TODO - Check what else we need to pass as arguments...
            //  * An "IR builder"? (factory to create objects that can easily be mapped to iQoala)
            //  * A map between the original op and the iQoalaMC object?
            return mlir::failure();
        }
    };

    class QoalaTranslationInterfaces
            : public mlir::DialectInterfaceCollection<QoalaTranslationDialectInterface> {
    public:
        using Base::Base;
        explicit QoalaTranslationInterfaces(mlir::MLIRContext *ctx) : DialectInterfaceCollection(ctx) {}

        /// Translates the given operation to iQoala using the interface implemented
        /// by the op's dialect.
        virtual mlir::LogicalResult convertOperation(mlir::Operation *op, translate::ModuleTranslation &moduleTranslation) const {
            if (const QoalaTranslationDialectInterface *iface = getInterfaceFor(op))
                return iface->convertOperation(op, moduleTranslation);
            return mlir::failure();
        }
    };
} // namespace qoala::iqoala

#endif //QOALATRANSLATIONINTERFACE_H
