#pragma once
#include <cstdint>

namespace Vpx {

enum class Gen {
    Vp8,
    Vp9,
};

struct Plane {
    uint8_t* data;
    const unsigned int height;
    const unsigned int width;
    const int stride;
};

}  // namespace Vpx
