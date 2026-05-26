cmake_minimum_required(VERSION 3.28)
include(cmake/DynamicVersion.cmake)

detect_project_version()
message(STATUS "Detected version: '${PROJECT_VERSION}'")

project(qoala-mlir VERSION ${PROJECT_VERSION} LANGUAGES CXX C)

set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(MLIR REQUIRED CONFIG)
find_package(SCIP REQUIRED CONFIG
             PATHS "${SCIP_DIR}"
             NO_DEFAULT_PATH)

# Manually extract SCIP include dir (if not already exposed by SCIP package)
get_target_property(SCIP_INCLUDE_DIR libscip INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "SCIP include dir: ${SCIP_INCLUDE_DIR}")

message(STATUS "Using MLIRConfig.cmake in: ${MLIR_DIR}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/bin)
set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/lib)
set(MLIR_BINARY_DIR ${CMAKE_BINARY_DIR})
# We set LLVM_BUILD_TOOLS to ON so "tool" executables
# are added to the "all" target
set(LLVM_BUILD_TOOLS ON)

list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(TableGen)
include(AddLLVM)
include(AddMLIR)
include(HandleLLVMOptions)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${MLIR_INCLUDE_DIRS})
include_directories(${SCIP_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_BINARY_DIR}/include)
link_directories(${LLVM_BUILD_LIBRARY_DIR})
add_definitions(${LLVM_DEFINITIONS})

add_subdirectory(lib)
add_subdirectory(include)
# add_subdirectory(test)
add_subdirectory(tools)

# Generate a version header so C++ can query the version at runtime
configure_file(
        "${CMAKE_SOURCE_DIR}/include/version.h.in"
        "${CMAKE_BINARY_DIR}/include/version.h"
        @ONLY
)

# Install the qoala-opt executable
install(
        TARGETS qoala-opt
        EXCLUDE_FROM_ALL
        COMPONENT qoala-opt-wheel
        RUNTIME DESTINATION ${PY_BUILD_CMAKE_PROJECT_NAME}-${PY_BUILD_CMAKE_PROJECT_VERSION}.data/scripts
        LIBRARY DESTINATION ${PY_BUILD_CMAKE_PROJECT_NAME}-${PY_BUILD_CMAKE_PROJECT_VERSION}.data/platlib
)
# Install the qoala-opt executable
install(
        TARGETS qoala-translate
        EXCLUDE_FROM_ALL
        COMPONENT qoala-translate-wheel
        RUNTIME DESTINATION ${PY_BUILD_CMAKE_PROJECT_NAME}-${PY_BUILD_CMAKE_PROJECT_VERSION}.data/scripts
        LIBRARY DESTINATION ${PY_BUILD_CMAKE_PROJECT_NAME}-${PY_BUILD_CMAKE_PROJECT_VERSION}.data/platlib
)
