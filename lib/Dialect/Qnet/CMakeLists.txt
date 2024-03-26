add_mlir_dialect_library(MLIRQnet
        QnetDialect.cpp
        Qnet.cpp
        QnetTypes.cpp

    ADDITIONAL_HEADER_DIRS
    ${PROJECT_SOURCE_DIR}/include/Dialect/Qnet

    DEPENDS
    MLIRQnetIncGen
    MLIRQnetPassIncGen

    LINK_LIBS PUBLIC
    MLIRIR
    MLIRInferTypeOpInterface
    MLIRFuncDialect

    LINK_LIBS PRIVATE
    MLIRAnalysis
    MLIRParser
    MLIRSideEffectInterfaces
    MLIRTransforms
)
