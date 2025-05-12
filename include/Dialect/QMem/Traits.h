#ifndef QOALA_MLIR_QMEM_TRAITS_H
#define QOALA_MLIR_QMEM_TRAITS_H
#include "mlir/IR/OpDefinition.h"

namespace mlir::OpTrait {
    /**
     * Declaration and definition of the "Entangle" op trait used in the QMem ops that
     * perform an entanglement operation.
     * WARNING: This class _must_ be defined in the mlir::OpTrait namespace.
     */
    template<typename ConcreteType>
    class Entangle : public mlir::OpTrait::TraitBase<ConcreteType, Entangle> {
        // In the meantime, we don't have any specific behavior for operation
        // with the "Entangle" trait
    };
}

#endif //QOALA_MLIR_QMEM_TRAITS_H
