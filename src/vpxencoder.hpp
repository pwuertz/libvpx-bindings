#pragma once
#include <memory>
#include <cstdint>
#include <functional>
#include "vpxcommon.hpp"

namespace Vpx {

struct Encoder
{
private:
    struct Ctx;

public:
    struct Config {
        unsigned int width;
        unsigned int height;
        unsigned int fps = 30;
        unsigned int bitrate = 3000;
        unsigned int threads = 0;
        int cpu_used = 16;
        Gen gen = Gen::Vp8;
    };

    Encoder(const Config& config);
    ~Encoder();

    Plane yPlane() const noexcept;

    void copyFromGray(const uint8_t* data);

    using packet_handler_t = std::function<void(const uint8_t*, size_t)>;
    void encode(const packet_handler_t& fn);

private:
    std::unique_ptr<Ctx> ctx;
};

}  // namespace Vpx
