add_mlir_translation_library(MLIRControlFlowToiQoalaTranslation
        ControlFlowToiQoalaTranslation.cpp

        LINK_COMPONENTS
        Core

        LINK_LIBS PUBLIC
        QoalaDialectHelpers
        MLIRTargetiQoalaExport
        MLIRIR
        MLIRSupport
)