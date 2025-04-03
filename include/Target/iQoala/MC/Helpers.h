#ifndef TARGET_TRANSLATION_HELPERS_H
#define TARGET_TRANSLATION_HELPERS_H

#include "Target/iQoala/ModuleTranslation.h"
#include "mlir/IR/Value.h"

namespace qoala::iqoala::helpers {
    /* Template to easily invoke the creation of an MC instruction of a given type
     * We need this template, since it will be instantiated in the DialectToiQoalaTranslation
     * sources. We can't move this template to iQoalaMC, since the allocation of a registry
     * invokes a method from the MLIRTargetiQoalaExport library. If we place this code
     * in the iQoalaMC.h header, it would lead to a cyclic dependency in the project.
     */
    template <typename InstrType>
    InstrType *buildInstruction(translate::ModuleTranslation *moduleTranslation,
                        mlir::Operation *mlirOperation,
                        typename InstrType::OpCode opCode,
                        const std::optional<mlir::Value> result,
                        const std::optional<assembly::iQoalaRegType> resultRegType,
                        mlir::SmallVector<assembly::iQoalaMCOperand *>extraOperands,
                        const bool useOpOperands = true, const bool appendInstruction = true){
        std::optional<assembly::iQoalaRegReference *>resRegRef;
        if (resultRegType.has_value()) {
            const uint8_t regNumber = moduleTranslation->getQoalaModule()->getiQoalaContext()->allocateRegister(resultRegType.value());
            resRegRef = assembly::iQoalaRegReference::createRegReference(resultRegType.value(), regNumber);
        } else {
            resRegRef = std::nullopt;
        }

        return InstrType::build(
            moduleTranslation, mlirOperation,
            result, resRegRef,
            opCode, extraOperands,
            useOpOperands, appendInstruction);
    }
}

#endif //TARGET_TRANSLATION_HELPERS_H
