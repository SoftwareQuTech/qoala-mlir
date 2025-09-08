add_mlir_dialect_library(MLIRQNet
        QNetDialect.cpp
        QNet.cpp
        QNetTypes.cpp

        ADDITIONAL_HEADER_DIRS
        ${PROJECT_SOURCE_DIR}/include/Dialect/QNet

        DEPENDS
        MLIRQNetIncGen
        MLIRQNetPassIncGen

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
