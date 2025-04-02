add_mlir_translation_library(MLIRQoalaHostToiQoalaTranslation
        QoalaHostToiQoalaTranslation.cpp

        LINK_COMPONENTS
        Core

        LINK_LIBS PUBLIC
        MLIRTargetiQoalaExport
        MLIRNetQASMAnalysis
        MLIRIR
        MLIRQoalaHost
        MLIRSupport
)