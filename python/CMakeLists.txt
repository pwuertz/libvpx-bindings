cmake_minimum_required(VERSION 3.16)
project(pylibvpx LANGUAGES CXX)

# Default to Release build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Fetch conan.cmake
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
       "${CMAKE_BINARY_DIR}/conan.cmake" TLS_VERIFY ON)
endif()
include(${CMAKE_BINARY_DIR}/conan.cmake)

# Get pybind11 and libvpx from conan
conan_cmake_run(
  REQUIRES pybind11/2.6.2 libvpx/1.9.0
  BASIC_SETUP
  CMAKE_TARGETS
  BUILD missing
)

# Create libvpx python library
pybind11_add_module(${PROJECT_NAME} "pylibvpx.cpp" "vpxcommon.hpp" "vpxcommon_impl.hpp")
  set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
)
target_link_libraries(${PROJECT_NAME} PRIVATE CONAN_PKG::pybind11 CONAN_PKG::libvpx)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=pylibvpx MODULE_VERSION=0.1)

option(WITH_VPX_ENCODER "With vpx encoder" ON)
if(WITH_VPX_ENCODER)
  message(STATUS "With VPX encoder")
  target_sources(${PROJECT_NAME} PRIVATE "vpxencoder.hpp" "vpxencoder.cpp")
  target_compile_definitions(${PROJECT_NAME} PRIVATE WITH_VPX_ENCODER=1)
endif()

option(WITH_VPX_DECODER "With vpx decoder" ON)
if(WITH_VPX_DECODER)
  message(STATUS "With VPX decoder")
  target_sources(${PROJECT_NAME} PRIVATE "vpxdecoder.hpp" "vpxdecoder.cpp")
  target_compile_definitions(${PROJECT_NAME} PRIVATE WITH_VPX_DECODER=1)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  target_compile_options(${PROJECT_NAME} PUBLIC "/Zc:__cplusplus")
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION ".")
