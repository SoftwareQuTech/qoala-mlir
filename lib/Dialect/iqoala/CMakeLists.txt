add_mlir_dialect_library(MLIRiQoala
    iQoalaDialect.cpp
    iQoala.cpp

    DEPENDS
    MLIRiQoalaIncGen

    LINK_LIBS PUBLIC
    MLIRIR

    LINK_LIBS PRIVATE
    MLIRAnalysis
    MLIRIR
    MLIRParser
    MLIRSideEffectInterfaces
    MLIRTransforms
)

add_subdirectory(Transforms)