#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/numpy.h>
#include "vpxcommon.hpp"
#ifdef WITH_VPX_ENCODER
#include "vpxencoder.hpp"
#endif
#ifdef WITH_VPX_DECODER
#include "vpxdecoder.hpp"
#endif

#include <string_view>
#include <iostream>

namespace py = pybind11;

constexpr std::string_view VERSION_STR = PYBIND11_TOSTRING(MODULE_VERSION);

#ifdef WITH_VPX_ENCODER
void py_vpxe_encode(Vpx::Encoder& encoder, const py::object& fn)
{
    const auto packetHandler = [&](const uint8_t* packet, const size_t size) {
        fn(py::memoryview::from_memory(packet, size));
    };
    encoder.encode(packetHandler);
}
py::array_t<uint8_t, py::array::c_style> py_vpxe_yplane(Vpx::Encoder& encoder)
{
    const Vpx::Plane plane = encoder.yPlane();
    return py::array_t<uint8_t, py::array::c_style>(
        std::vector<Py_ssize_t>{plane.height, plane.width},
        std::vector<Py_ssize_t>{plane.stride, 1},
        plane.data
    );
}
void py_vpxe_copygray(Vpx::Encoder& encoder, const py::array& image)
{
    const auto* imageData = static_cast<const uint8_t*>(image.data());
    encoder.copyFromGray(imageData);
}
#endif

#ifdef WITH_VPX_DECODER
void py_vpxd_decode(Vpx::Decoder& decoder, const py::bytes& packet, const py::object& fn)
{
    uint8_t* data; Py_ssize_t size;
    PyBytes_AsStringAndSize(packet.ptr(), reinterpret_cast<char**>(&data), &size);
    const auto frameHandler = [&](const Vpx::Plane& plane) {
        py::array_t<uint8_t, py::array::c_style> img(
            std::vector<Py_ssize_t>{plane.height, plane.width},
            std::vector<Py_ssize_t>{plane.stride, 1},
            plane.data
        );
        fn(img);
    };
    decoder.decode(data, static_cast<size_t>(size), frameHandler);
}
#endif


PYBIND11_MODULE(MODULE_NAME, m) {
    m.attr("__version__") = py::str(VERSION_STR.data(), VERSION_STR.size());

    py::enum_<Vpx::Gen>(m, "VpxGen")
    .value("Vp8", Vpx::Gen::Vp8)
    .value("Vp9", Vpx::Gen::Vp9);

#ifdef WITH_VPX_ENCODER
    py::class_<Vpx::Encoder> vpxencCls(m, "VpxEncoder");
    vpxencCls.def(py::init<const Vpx::Encoder::Config&>())
    .def(py::init([](const unsigned int width, const unsigned int height) {
        return new Vpx::Encoder({.width = width, .height = height});
    }))
    .def("encode", py_vpxe_encode)
    .def("yPlane", py_vpxe_yplane)
    .def("copyGray", py_vpxe_copygray);

    py::class_<Vpx::Encoder::Config>(vpxencCls, "Config")
    .def(py::init([](const unsigned int width, const unsigned int height) {
        return new Vpx::Encoder::Config { .width=width, .height=height };
    }))
    .def_readwrite("width", &Vpx::Encoder::Config::width)
    .def_readwrite("height", &Vpx::Encoder::Config::height)
    .def_readwrite("fps", &Vpx::Encoder::Config::fps)
    .def_readwrite("bitrate", &Vpx::Encoder::Config::bitrate)
    .def_readwrite("threads", &Vpx::Encoder::Config::threads)
    .def_readwrite("cpu_used", &Vpx::Encoder::Config::cpu_used)
    .def_readwrite("gen", &Vpx::Encoder::Config::gen);
#endif

#ifdef WITH_VPX_DECODER
    py::class_<Vpx::Decoder> vpxdecCls(m, "VpxDecoder");
    vpxdecCls.def(py::init<Vpx::Gen>())
    .def("decode", py_vpxd_decode);
#endif
}
