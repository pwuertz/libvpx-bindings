include(ExternalProject)

find_program(EMCONFIGURE NAMES "emconfigure" PATHS "${EMSCRIPTEN_ROOT_PATH}" REQUIRED)
find_program(EMMAKE NAMES "emmake" PATHS "${EMSCRIPTEN_ROOT_PATH}" REQUIRED)

set(LIBVPX_SRC_PATH "${CMAKE_BINARY_DIR}/libvpx-src")
set(LIBVPX_BIN_PATH "${CMAKE_BINARY_DIR}/libvpx-build")
set(LIBVPX_INSTALL_PATH "${CMAKE_BINARY_DIR}/libvpx")

list(APPEND LIBVPX_CONFIGURE_COMMAND
    "STRIP=touch" "${EMCONFIGURE}" "${LIBVPX_SRC_PATH}/configure"
    "--target=generic-gnu" "--disable-multithread"
    "--disable-examples" "--disable-tools" "--disable-docs"
    "--disable-unit-tests" "--disable-webm-io" "--disable-libyuv"
    "--enable-vp8-decoder" "--enable-vp9-decoder"
    "--disable-vp8-encoder" "--disable-vp9-encoder"
    "--enable-realtime-only"
    "--extra-cflags=-s WASM=1 -DWASM -I${EMSCRIPTEN_ROOT_PATH}/system/lib/libcxxabi/include"
    "--prefix=${LIBVPX_INSTALL_PATH}"
    )
list(APPEND LIBVPX_BUILD_COMMAND
    "${EMMAKE}" "make" "-j4"
    )
list(APPEND LIBVPX_INSTALL_COMMAND
    "make" "install"
    )

ExternalProject_Add(libvpx_external
    GIT_REPOSITORY "https://github.com/webmproject/libvpx.git"
    GIT_TAG "v1.9.0"
    GIT_SHALLOW ON
    UPDATE_COMMAND ""
    SOURCE_DIR ${LIBVPX_SRC_PATH}
    BINARY_DIR ${LIBVPX_BIN_PATH}
    INSTALL_DIR ${LIBVPX_INSTALL_PATH}
    CONFIGURE_COMMAND ${LIBVPX_CONFIGURE_COMMAND}
    BUILD_COMMAND ${LIBVPX_BUILD_COMMAND}
    INSTALL_COMMAND ${LIBVPX_INSTALL_COMMAND}
    BUILD_BYPRODUCTS "${LIBVPX_INSTALL_PATH}/lib/libvpx.a"
    )

add_library(libvpx STATIC IMPORTED GLOBAL)
add_dependencies(libvpx libvpx_external)
file(MAKE_DIRECTORY "${LIBVPX_INSTALL_PATH}/include")
set_target_properties(libvpx PROPERTIES
    IMPORTED_LOCATION "${LIBVPX_INSTALL_PATH}/lib/libvpx.a"
    INTERFACE_INCLUDE_DIRECTORIES "${LIBVPX_INSTALL_PATH}/include"
    )
