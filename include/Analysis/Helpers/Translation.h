#ifndef QOALA_MLIR_TRANSLATION_H
#define QOALA_MLIR_TRANSLATION_H

#include "mlir/IR/DialectRegistry.h"

namespace qoala::translate {
    /**
     * Helper template used to create methods for registering dialect translations.
     * @tparam DialectTy Dialect class to translate
     * @tparam TranslationTy Class implementing the translation
     * @param registry The DialectRegistry instance to register the new translation class
     */
    template<typename DialectTy, typename TranslationTy>
    void registeriQoalaTranslation(mlir::DialectRegistry &registry) {
        registry.insert<DialectTy>();
        registry.addExtension(
                +[](mlir::MLIRContext *ctx, DialectTy *dialect) { dialect->template addInterfaces<TranslationTy>(); });
    }
} // namespace qoala::translate

#endif // QOALA_MLIR_TRANSLATION_H
