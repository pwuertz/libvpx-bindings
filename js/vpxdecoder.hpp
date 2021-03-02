#pragma once
#include <memory>
#include <cstdint>
#include <functional>
#include "vpxcommon.hpp"

namespace Vpx {

struct Decoder
{
private:
    struct Ctx;

public:
    Decoder(Gen gen = Gen::Vp8);
    ~Decoder();

    using frame_handler_t = std::function<void(const Plane&)>;
    void decode(const uint8_t* packet, size_t size, const frame_handler_t& fn);

private:
    std::unique_ptr<Ctx> ctx;
};

}  // namespace Vpx
