add_mlir_translation_library(MLIRQRemoteToiQoalaTranslation
        QRemoteToiQoalaTranslation.cpp

        LINK_COMPONENTS
        Core

        LINK_LIBS PUBLIC
        MLIRIR
        MLIRQRemote
        MLIRSupport
)