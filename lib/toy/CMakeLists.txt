add_custom_target(Toy)

set_target_properties(Toy PROPERTIES FOLDER Examples)
macro(add_toy_chapter name)
  add_dependencies(Toy ${name})
  add_llvm_example(${name} ${ARGN})
endmacro(add_toy_chapter name)

set(LLVM_LINK_COMPONENTS
  Support
  )

add_toy_chapter(toyc-ch2
  toyc.cpp
  AST.cpp
  MLIRGen.cpp
  Dialect.cpp

  DEPENDS
  ToyCh2OpsIncGen

  )
include_directories(include/)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include/)
target_link_libraries(toyc-ch2
  PRIVATE
    MLIRAnalysis
    MLIRIR
    MLIRParser
    MLIRSideEffectInterfaces
    MLIRTransforms)
