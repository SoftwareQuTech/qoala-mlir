add_mlir_dialect(Toy toy)

get_property(target_names DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
message("targets from include/: ${target_names}")