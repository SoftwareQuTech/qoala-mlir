add_mlir_library(QoalaDialectHelpers
        DialectHelpers.cpp

        DEPENDS
        MLIRQoalaHostIncGen

        LINK_COMPONENTS
        Core

        LINK_LIBS PUBLIC
        #MLIRQMemIncGen
        MLIRNetQASM
        MLIRIR
)

target_include_directories(QoalaDialectHelpers
        PRIVATE include/Dialect/Helpers
)