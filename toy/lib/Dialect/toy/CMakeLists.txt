add_mlir_dialect_library(MLIRToy
    ToyDialect.cpp    
    Toy.cpp

    DEPENDS
    MLIRToyIncGen

    LINK_LIBS PUBLIC
    MLIRIR
)

get_property(target_names DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
message("targets from lib/: ${target_names}")