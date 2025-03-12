#ifndef TARGET_TRANSLATION_HELPERS_H
#define TARGET_TRANSLATION_HELPERS_H

#include "Target/iQoala/MC/iQoalaMC.h"
#include "Target/iQoala/ModuleTranslation.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/Value.h"

namespace qoala::iqoala::helpers {
    /* Template to easily invoke the creation of an MC instruction of a given type
     * We need this template, since it will be instantiated in the DialectToiQoalaTranslation
     * sources We can't move this template to iQoalaMC, since the allocation of a registry
     * invokes a method from the MLIRTargetiQoalaExport library. If we place this code
     * in the iQoalaMC.h header, it forces us to insert a cyclic dependency in the project.
     */
    template <typename InstrType>
    mlir::LogicalResult buildInstruction(translate::ModuleTranslation *moduleTranslation,
                        mlir::Operation *mlirOperation,
                        typename InstrType::OpCode opCode,
                        const std::optional<mlir::Value>result,
                        const std::optional<assembly::iQoalaRegType> resultType,
                        const std::optional<assembly::iQoalaMCOperand *>immediateVal){
        llvm::SmallVector<assembly::iQoalaMCOperand *> mcOperands;
        if (immediateVal.has_value()) {
            mcOperands.push_back(immediateVal.value());
        }

        std::optional<assembly::iQoalaRegReference *>resRegRef;
        if (resultType.has_value()) {
            const uint8_t regNumber = moduleTranslation->getQoalaModule()->getiQoalaContext()->allocateRegister(resultType.value());
            resRegRef = assembly::iQoalaRegReference::createRegReference(resultType.value(), regNumber);
        } else {
            resRegRef = std::nullopt;
        }

        const auto newAssign = assembly::InstructionBuilder::build<InstrType>(
            moduleTranslation, mlirOperation,
            result, resRegRef,
            opCode, mcOperands);
        return newAssign ? mlir::success() : mlir::failure();
    }
}

#endif //TARGET_TRANSLATION_HELPERS_H
