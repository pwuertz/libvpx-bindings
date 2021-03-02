#include <string>
#include <iostream>
#include <chrono>
#include <optional>
#include <emscripten/bind.h>
#include "vpxdecoder.hpp"

namespace em = emscripten;

namespace {

struct RgbaBuffer {
    RgbaBuffer(unsigned int width, unsigned int height)
        : width(width), height(height)
        , rgba(width * height)
        , imageDataView(imageDataViewFromBuffer(width, height, rgba))
    { }
    RgbaBuffer(const RgbaBuffer &) = delete;
    RgbaBuffer &operator=(const RgbaBuffer &) = delete;

public:
    const unsigned int width;
    const unsigned int height;
    std::vector<uint32_t> rgba;
    em::val imageDataView;

private:
    static em::val imageDataViewFromBuffer(
        unsigned int width, unsigned int height, std::vector<uint32_t> &rgba)
    {
        // Getting from c++ buffer to js ImageData without a copy:
        // U8 view -> U8clamped from U8 -> ImageData from U8clamped
        const auto viewU8 = em::val{em::typed_memory_view(
            width * height * 4, reinterpret_cast<uint8_t *>(rgba.data()))};
        const auto viewU8C = em::val::global("Uint8ClampedArray").new_(
            viewU8["buffer"], viewU8["byteOffset"], viewU8["byteLength"]);
        return em::val::global("ImageData").new_(viewU8C, width, height);
    }
};

struct Decoder {
    Decoder()
        : packetBuffer(1 * 1024 * 1024)
        , decoder(Vpx::Gen::Vp8)
    { }

    bool decode(const emscripten::val &u8array)
    {
        // Copy bytes from JS array to C++ buffer
        const auto size = u8array["byteLength"].as<std::size_t>();
        if (packetBuffer.size() < size) {
            packetBuffer.resize(size);
        }
        const auto packetBufferView = em::val{em::typed_memory_view(
            size, packetBuffer.data())};
        packetBufferView.call<void>("set", u8array);

        // Decode packet to RGBA frame buffer
        bool imageDecoded = false;
        decoder.decode(packetBuffer.data(), size, [&](const Vpx::Plane &plane) {
            // Initialize / resize RGBA buffer as required
            const bool needNewBuffer =
                !rgbaBuffer
                || (plane.width != rgbaBuffer->width)
                || (plane.height != rgbaBuffer->height);
            if (needNewBuffer) {
                rgbaBuffer.emplace(plane.width, plane.height);
            }
            // Copy yPlane to RGBA buffer
            uint32_t *rgba = rgbaBuffer->rgba.data();
            for (unsigned int iy = 0; iy < plane.height; ++iy) {
                for (unsigned int ix = 0; ix < plane.width; ++ix) {
                    const uint8_t val = plane.data[iy * plane.stride + ix];
                    rgba[iy * plane.width + ix] =
                        uint32_t(0xff000000) | uint32_t(val << 16) |
                        uint32_t(val << 8) | uint32_t(val << 0);
                }
            }
            imageDecoded = true;
        });
        return imageDecoded;
    }

    const em::val &frame() const { return rgbaBuffer->imageDataView; }

private:
    std::vector<uint8_t> packetBuffer;
    Vpx::Decoder decoder;
    std::optional<RgbaBuffer> rgbaBuffer;
};

}  // namespace

EMSCRIPTEN_BINDINGS(module) {
    emscripten::class_<Decoder>("Decoder")
        .constructor<>()
        .property("frame", &Decoder::frame)
        .function("decode", &Decoder::decode);
}