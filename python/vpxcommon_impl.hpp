#pragma once
#include <algorithm>
#include <stdexcept>
#include <vpx/vpx_image.h>
#include "vpxcommon.hpp"

namespace Vpx {

struct Image
{
    Image(const unsigned int width, const unsigned int height)
    {
        if (vpx_img_alloc(&m_img, VPX_IMG_FMT_I420, width, height, 1) == nullptr) {
            throw std::runtime_error("Failed to allocate image");
        }
        fillBlack();
    }

    Image(const Image&) = delete;
    void operator=(const Image&) = delete;

    ~Image() noexcept { vpx_img_free(&m_img); }

    constexpr operator vpx_image_t*() noexcept { return &m_img; }
    constexpr operator const vpx_image_t*() const noexcept { return &m_img; }

    void fillBlack() noexcept
    {
        std::fill_n(m_img.planes[VPX_PLANE_Y], m_img.w * m_img.h, 0);
        std::fill_n(m_img.planes[VPX_PLANE_U], m_img.w * m_img.h / 4, 128);
        std::fill_n(m_img.planes[VPX_PLANE_V], m_img.w * m_img.h / 4, 128);
    }

    constexpr Plane yPlane() const noexcept
    {
        return {
            .data = m_img.planes[VPX_PLANE_Y],
            .height = m_img.d_h, .width = m_img.d_w,
            .stride = m_img.stride[0]
        };
    }

    void copyFromGrayscale(const uint8_t* src)
    {
        const auto yPlane = this->yPlane();
        for (unsigned int row = 0; row < yPlane.height; ++row) {
            const auto* srcRow = src + row * yPlane.width;
            auto* dstRow = yPlane.data + row * yPlane.stride;
            std::copy_n(srcRow, yPlane.width, dstRow);
        }
    }

private:
    vpx_image_t m_img;
};

}  // namespace Vpx
