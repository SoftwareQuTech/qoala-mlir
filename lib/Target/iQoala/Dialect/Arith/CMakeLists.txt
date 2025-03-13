add_mlir_translation_library(MLIRArithToiQoalaTranslation
        ArithToiQoalaTranslation.cpp

        LINK_COMPONENTS
        Core

        LINK_LIBS PUBLIC
        QoalaDialectHelpers
        MLIRTargetiQoalaExport
        MLIRIR
        MLIRArithDialect
        MLIRSupport
)