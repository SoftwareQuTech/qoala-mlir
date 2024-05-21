#ifndef QOALATRANSLATIONINTERFACE_H
#define QOALATRANSLATIONINTERFACE_H

#include "mlir/IR/DialectInterface.h"
#include "mlir/Support/LogicalResult.h"

using namespace mlir;

class QoalaTranslationDialectInterface
        : public DialectInterface::Base<QoalaTranslationDialectInterface> {
public:
    explicit QoalaTranslationDialectInterface(Dialect *dialect) : Base(dialect) {}

    /// Hook for derived dialect interface to provide translation of the
    /// operations to iQoala
    virtual LogicalResult
    convertOperation(Operation *op) const {
        // TODO - Check what else we need to pass as arguments...
        //  * An "IR builder"? (factory to create objects that can easily be mapped to iQoala)
        //  * A map between the original op and the iQoala object?
        return failure();
    }
};

#endif //QOALATRANSLATIONINTERFACE_H
