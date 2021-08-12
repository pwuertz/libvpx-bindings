cmake_minimum_required(VERSION 3.16)

# Default to Release build type
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

project(libvpxjs CXX)

if(NOT DEFINED EMSCRIPTEN)
  message(FATAL_ERROR "Project requires EMSCRIPTEN toolchain")
endif()

# Include libvpx library definition
add_subdirectory("libvpx")

# Create libvpxjs WASM library
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
  "libvpxjs.cpp"
  "vpxcommon.hpp"
  "vpxcommon_impl.hpp"
  "vpxdecoder.hpp"
  "vpxdecoder.cpp")

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(${PROJECT_NAME} PRIVATE "${EMSCRIPTEN_ROOT_PATH}/system/include")
set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  SUFFIX ".esm.js"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

list(APPEND WASM_ES6MOD_LINK_FLAGS
  "--bind"
  "-s WASM=1"
  "-s DISABLE_EXCEPTION_CATCHING=0"
  "-s MODULARIZE=1"
  "-s EXPORT_ES6=1"
  "-s EXPORT_NAME=\"${PROJECT_NAME}\"")

target_link_libraries(${PROJECT_NAME} PRIVATE ${WASM_ES6MOD_LINK_FLAGS} libvpx)
