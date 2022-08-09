add_mlir_dialect_library(MLIRToy
    ToyDialect.cpp    
    Toy.cpp

    DEPENDS
    MLIRToyIncGen

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