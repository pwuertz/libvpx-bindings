cmake_minimum_required(VERSION 3.24)

if(NOT SKBUILD)
    message(FATAL_ERROR "Build should be driven by scikit-build-core")
endif()
set(MODULE_NAME "_${SKBUILD_PROJECT_NAME}")
set(MODULE_VERSION ${SKBUILD_PROJECT_VERSION})
project(${MODULE_NAME} VERSION ${MODULE_VERSION} LANGUAGES CXX)


# Dependencies

find_package(
    Python 3.11 REQUIRED
    COMPONENTS Interpreter Development.Module
    OPTIONAL_COMPONENTS Development.SABIModule
)
find_package(nanobind CONFIG REQUIRED)
find_package(libvpx REQUIRED)


# Define extension module

nanobind_add_module(
  ${MODULE_NAME}
  STABLE_ABI
  NB_STATIC
  "src/pylibvpx.cpp"
  "src/vpxcommon.hpp"
  "src/vpxcommon_impl.hpp"
  "src/vpxencoder.hpp"
  "src/vpxencoder.cpp"
  "src/vpxdecoder.hpp"
  "src/vpxdecoder.cpp"
)

set_target_properties(
    ${MODULE_NAME}
    PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON
)
target_link_libraries(${MODULE_NAME} PRIVATE libvpx::libvpx)
target_compile_definitions(${MODULE_NAME} PRIVATE MODULE_NAME=${MODULE_NAME})
target_compile_definitions(
    ${MODULE_NAME}
    PRIVATE MODULE_VERSION=${MODULE_VERSION}
)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${MODULE_NAME} PRIVATE -Wall -Wextra)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${MODULE_NAME} PUBLIC "/Zc:__cplusplus")
endif()


# Generate typing stubs from extension

nanobind_add_stub(
  ${MODULE_NAME}_stub
  MODULE ${MODULE_NAME}
  OUTPUT ${MODULE_NAME}.pyi
  PYTHON_PATH $<TARGET_FILE_DIR:${MODULE_NAME}>
  DEPENDS ${MODULE_NAME}
  MARKER_FILE py.typed
)


# Install to scikit-build target

install(
    TARGETS
        ${MODULE_NAME}
    LIBRARY
    DESTINATION ${SKBUILD_PROJECT_NAME}
)
install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/__init__.py
        ${${PROJECT_NAME}_BINARY_DIR}/py.typed
        ${${PROJECT_NAME}_BINARY_DIR}/${MODULE_NAME}.pyi
    DESTINATION ${SKBUILD_PROJECT_NAME}
)
