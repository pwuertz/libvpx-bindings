cmake_minimum_required(VERSION 3.16)
project(pylibvpx LANGUAGES CXX)

# Use LTO for building this module
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

find_package(
  Python 3.9 REQUIRED
  COMPONENTS Interpreter Development.Module
  OPTIONAL_COMPONENTS Development.SABIModule
)

# Fetch conan.cmake
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
       "${CMAKE_BINARY_DIR}/conan.cmake" TLS_VERIFY ON)
endif()
include(${CMAKE_BINARY_DIR}/conan.cmake)

# Fetch nanobind library
include(FetchContent)
FetchContent_Declare(
  nanobind
  GIT_REPOSITORY https://github.com/wjakob/nanobind
  GIT_TAG        c5454462e35f29310df05b412b5c48997d634bdd
)
FetchContent_MakeAvailable(nanobind)

# Get libvpx from conan
conan_cmake_configure(
  REQUIRES libvpx/1.14.0
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
nanobind_add_module(${PROJECT_NAME}
  STABLE_ABI
  "pylibvpx.cpp"
  "vpxcommon.hpp"
  "vpxcommon_impl.hpp"
)
nanobind_add_stub(
  ${PROJECT_NAME}_stub
  MODULE ${PROJECT_NAME}
  OUTPUT ${PROJECT_NAME}.pyi
  PYTHON_PATH $<TARGET_FILE_DIR:${PROJECT_NAME}>
  DEPENDS ${PROJECT_NAME}
  MARKER_FILE py.typed
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
)

find_package(libvpx REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE libvpx::libvpx)

target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
if(DEFINED SKBUILD_PROJECT_VERSION)
  target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_VERSION=${SKBUILD_PROJECT_VERSION})
endif()

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
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  target_compile_options(${PROJECT_NAME} PUBLIC "/Zc:__cplusplus")
endif()

install(TARGETS ${PROJECT_NAME}
  DESTINATION "${PROJECT_NAME}"
)
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/__init__.py
  ${${PROJECT_NAME}_BINARY_DIR}/py.typed
  ${${PROJECT_NAME}_BINARY_DIR}/${PROJECT_NAME}.pyi
  DESTINATION "${PROJECT_NAME}"
)
