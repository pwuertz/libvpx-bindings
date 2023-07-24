cmake_minimum_required(VERSION 3.16)
project(pylibvpx LANGUAGES CXX VERSION "1.0.1")

# Default to Release build type
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

# Fetch conan.cmake
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
       "${CMAKE_BINARY_DIR}/conan.cmake" TLS_VERIFY ON)
endif()
include(${CMAKE_BINARY_DIR}/conan.cmake)

if(WIN32)
  # Prevent pybind11 from sharing resources with other, potentially ABI incompatible modules
  # https://github.com/pybind/pybind11/issues/2898
  add_definitions(-DPYBIND11_COMPILER_TYPE="_${PROJECT_NAME}_abi")
endif()

# Get pybind11 and libvpx from conan
conan_cmake_configure(
  REQUIRES pybind11/2.10.4 libvpx/1.11.0
  GENERATORS cmake_find_package
)
conan_cmake_autodetect(settings)
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  # MSVC 2022 can't build libvpx -> donwload binary for MSVC 2019
  list(APPEND settings "libvpx:compiler.version=16")
endif()
conan_cmake_install(
  PATH_OR_REFERENCE .
  BUILD missing
  REMOTE conancenter
  SETTINGS ${settings}
)

# Create libvpx python library
find_package(pybind11 REQUIRED)
find_package(libvpx REQUIRED)
pybind11_add_module(${PROJECT_NAME} "pylibvpx.cpp" "vpxcommon.hpp" "vpxcommon_impl.hpp")
  set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
)
target_link_libraries(${PROJECT_NAME} PRIVATE libvpx::libvpx)
target_compile_definitions(${PROJECT_NAME} PRIVATE
  MODULE_NAME=${PROJECT_NAME}
  MODULE_VERSION=${PROJECT_VERSION}
)

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
