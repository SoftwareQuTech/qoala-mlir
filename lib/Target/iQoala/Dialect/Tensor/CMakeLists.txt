add_mlir_translation_library(MLIRTensorToiQoalaTranslation
        TensorToiQoalaTranslate.cpp

        LINK_COMPONENTS
        Core

        LINK_LIBS PUBLIC
        MLIRTargetiQoalaExport
        MLIRIR
        MLIRTensorDialect
        MLIRSupport
)