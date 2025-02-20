#ifndef QOALATRANSLATIONINTERFACE_H
#define QOALATRANSLATIONINTERFACE_H

#include "mlir/IR/DialectInterface.h"
#include "mlir/Support/LogicalResult.h"

namespace qoala::translate {
    // Forward declaration to avoid circular includes
    class ModuleTranslation;

    /**
     * Main interface for translation classes. These classes need to be registered using the
     */
    class QoalaTranslationDialectInterface
            : public mlir::DialectInterface::Base<QoalaTranslationDialectInterface> {
    public:
        explicit QoalaTranslationDialectInterface(mlir::Dialect *dialect) : Base(dialect) {}

        /// Hook for derived dialect interface to provide translation of the
        /// operations to iQoala
        virtual mlir::LogicalResult convertOperation(
                mlir::Operation *op, ModuleTranslation &moduleTranslation) const {
            // TODO - Check what else we need to pass as arguments...
            //  * An "IR builder"? (factory to create objects that can easily be mapped to iQoala)
            //  * A map between the original op and the iQoalaMC object?
            return mlir::failure();
        }

        static void registerInto(mlir::DialectRegistry &r) {
            // Nothing to do; implementor should re-define this function
        }
    };

    /**
     * Class modelling a collection of translation interfaces. This class gets automatically
     * populated when registering the translations.
     */
    class QoalaTranslationInterfaces
            : public mlir::DialectInterfaceCollection<QoalaTranslationDialectInterface> {
    public:
        using Base::Base;
        explicit QoalaTranslationInterfaces(mlir::MLIRContext *ctx) : DialectInterfaceCollection(ctx) { }
    };
} // namespace qoala::iqoala

#endif //QOALATRANSLATIONINTERFACE_H
