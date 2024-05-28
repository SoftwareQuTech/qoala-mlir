add_mlir_translation_library(MLIRNetQASMToiQoalaTranslation
        NetQASMToiQoalaTranslation.cpp

        LINK_COMPONENTS
        Core

        LINK_LIBS PUBLIC
        MLIRIR
        MLIRNetQASM
        MLIRSupport
)