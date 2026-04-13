cmake_minimum_required(VERSION 3.20)
project(evmlir LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

set(LLVM_EXTERNAL_LIT "/usr/local/bin/lit")

find_package(MLIR REQUIRED CONFIG)
find_package(GTest REQUIRED)
enable_testing()

message(STATUS "Using MLIRConfig.cmake in: ${MLIR_DIR}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

include(TableGen)
include(AddLLVM)
include(AddMLIR)
include(HandleLLVMOptions)

include_directories(${MLIR_INCLUDE_DIRS})
include_directories(${LLVM_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_BINARY_DIR}/include)

add_definitions(${LLVM_DEFINITIONS})

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(tools)

# revm-bridge: Rust static library wrapping revm for EVM execution in tests
set(REVM_BRIDGE_DIR ${CMAKE_SOURCE_DIR}/revm-bridge)
set(REVM_BRIDGE_LIB ${REVM_BRIDGE_DIR}/target/release/librevm_bridge.a)

add_custom_target(revm_bridge_build
  COMMAND cargo build --release
  WORKING_DIRECTORY ${REVM_BRIDGE_DIR}
  BYPRODUCTS ${REVM_BRIDGE_LIB}
)

add_library(revm_bridge STATIC IMPORTED GLOBAL)
set_target_properties(revm_bridge PROPERTIES IMPORTED_LOCATION ${REVM_BRIDGE_LIB})
add_dependencies(revm_bridge revm_bridge_build)

add_subdirectory(test)