add_mlir_translation_library(MLIRBuiltinToiQoalaTranslation
        BuiltinToiQoalaTranslation.cpp

        LINK_COMPONENTS
        Core

        LINK_LIBS PUBLIC
        MLIRTargetiQoalaExport
        MLIRIR
        MLIRSupport
)