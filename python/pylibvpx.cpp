#include <Python.h>
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include "vpxcommon.hpp"
#ifdef WITH_VPX_ENCODER
#include "vpxencoder.hpp"
#endif
#ifdef WITH_VPX_DECODER
#include "vpxdecoder.hpp"
#endif

#include <string_view>
#include <iostream>

namespace nb = nanobind;
using namespace nb::literals;

using np_uint8_array_t = nb::ndarray<nb::numpy, uint8_t, nb::c_contig>;

struct memoryview : public nb::object
{
    memoryview(const uint8_t* mem, const Py_ssize_t size)
        : nb::object(makeMemoryView(mem, size), nb::detail::steal_t{})
    { }

private:
    static PyObject* makeMemoryView(const uint8_t* mem, Py_ssize_t size)
    {
        auto* h = PyMemoryView_FromMemory(const_cast<char*>(reinterpret_cast<const char*>(mem)), size, PyBUF_READ);
        if (!h) {
            throw std::runtime_error("Failed creating memoryview");
        }
        return h;
    }
};

#ifdef WITH_VPX_ENCODER
void py_vpxe_encode(Vpx::Encoder& encoder, const nb::callable& fn)
{
    const auto packetHandler = [&](const uint8_t* packet, const size_t size) {
        fn(memoryview(packet, size));
    };
    encoder.encode(packetHandler);
}

np_uint8_array_t py_vpxe_yplane(Vpx::Encoder& encoder)
{
    const Vpx::Plane plane = encoder.yPlane();
    constexpr size_t ndims = 2;
    const size_t shape[ndims] = {plane.height, plane.width};
    const int64_t strides[ndims] = {plane.stride, 1};
    return np_uint8_array_t(
        plane.data, ndims, shape, nb::handle(), strides
    );
}

void py_vpxe_copygray(Vpx::Encoder& encoder, const np_uint8_array_t& image)
{
    const auto* imageData = static_cast<const uint8_t*>(image.data());
    encoder.copyFromGray(imageData);
}
#endif

#ifdef WITH_VPX_DECODER
void py_vpxd_decode(Vpx::Decoder& decoder, const nb::bytes& packet, const nb::callable& fn)
{
    uint8_t* data; Py_ssize_t size;
    PyBytes_AsStringAndSize(packet.ptr(), reinterpret_cast<char**>(&data), &size);
    const auto frameHandler = [&](const Vpx::Plane& plane) {
        constexpr size_t ndims = 2;
        const size_t shape[ndims] = {plane.height, plane.width};
        const int64_t strides[ndims] = {plane.stride, 1};
        np_uint8_array_t img(
            plane.data, ndims, shape, nb::handle(), strides
        );
        fn(img);
    };
    decoder.decode(data, static_cast<size_t>(size), frameHandler);
}
#endif


NB_MODULE(MODULE_NAME, m) {

    nb::enum_<Vpx::Gen>(m, "VpxGen")
        .value("Vp8", Vpx::Gen::Vp8)
        .value("Vp9", Vpx::Gen::Vp9);

#ifdef WITH_VPX_ENCODER
    nb::class_<Vpx::Encoder> vpxencCls(m, "VpxEncoder");
    vpxencCls
        .def("__init__", [](Vpx::Encoder* t, const Vpx::Encoder::Config& config) {
            new(t) Vpx::Encoder{config};
        })
        .def("__init__", [](Vpx::Encoder* t, const unsigned int width, const unsigned int height) {
            new(t) Vpx::Encoder({.width = width, .height = height});
        })
        .def("encode", py_vpxe_encode, "fn"_a)
        .def("yPlane", py_vpxe_yplane)
        .def("copyGray", py_vpxe_copygray, "image"_a);

    nb::class_<Vpx::Encoder::Config>(vpxencCls, "Config")
        .def("__init__", [](Vpx::Encoder::Config* t, const unsigned int width, const unsigned int height) {
            new(t) Vpx::Encoder::Config{.width = width, .height = height};
        })
        .def_rw("width", &Vpx::Encoder::Config::width)
        .def_rw("height", &Vpx::Encoder::Config::height)
        .def_rw("fps", &Vpx::Encoder::Config::fps)
        .def_rw("bitrate", &Vpx::Encoder::Config::bitrate)
        .def_rw("threads", &Vpx::Encoder::Config::threads)
        .def_rw("cpu_used", &Vpx::Encoder::Config::cpu_used)
        .def_rw("gen", &Vpx::Encoder::Config::gen);
#endif

#ifdef WITH_VPX_DECODER
    nb::class_<Vpx::Decoder> vpxdecCls(m, "VpxDecoder");
    vpxdecCls
        .def(nb::init<Vpx::Gen>())
        .def("decode", py_vpxd_decode);
#endif

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)
#ifdef MODULE_VERSION    
    m.attr("__version__") = MACRO_STRINGIFY(MODULE_VERSION);
#else
    m.attr("__version__") = "dev";
#endif
}
