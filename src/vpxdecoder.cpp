#include "vpxdecoder.hpp"
#include "vpxcommon_impl.hpp"
#include <algorithm>
#include <stdexcept>
#include <vpx/vpx_decoder.h>
#include <vpx/vp8dx.h>

using namespace Vpx;

namespace {

constexpr vpx_codec_iface_t* decoder_vp8 = &vpx_codec_vp8_dx_algo;
constexpr vpx_codec_iface_t* decoder_vp9 = &vpx_codec_vp9_dx_algo;

struct DecCtx
{
    DecCtx(const Gen gen)
    {
        // Create decoder context
        const auto* decoder_iface = (gen == Gen::Vp8) ? decoder_vp8 : decoder_vp9;
        if (vpx_codec_dec_init(&m_ctx, decoder_iface, nullptr, 0) != VPX_CODEC_OK) {
            throw std::runtime_error("Failed to create decoding context");
        }
    }

    DecCtx(const DecCtx&) = delete;
    void operator=(const DecCtx&) = delete;

    ~DecCtx() noexcept { vpx_codec_destroy(&m_ctx); }

    constexpr operator vpx_codec_ctx_t*() noexcept { return &m_ctx; }
    constexpr operator const vpx_codec_ctx_t*() const noexcept { return &m_ctx; }

private:
    vpx_codec_ctx_t m_ctx;
};

}  // private namespace


struct Decoder::Ctx {
    Ctx(const Gen gen) : m_ctx(gen) { }

    template<typename Fn> void decode(const uint8_t* data, const size_t size, Fn&& fn)
    {
        constexpr auto VPX_DL_REALTIME = 1;
        if (vpx_codec_decode(m_ctx, data, size, nullptr, VPX_DL_REALTIME) != VPX_CODEC_OK) {
            throw std::runtime_error("Failed to decode packet");
        }

        vpx_codec_iter_t iter = nullptr;
        vpx_image* img = nullptr;
        while ((img = vpx_codec_get_frame(m_ctx, &iter)) != nullptr) {
            fn(Plane {
                .data = img->planes[VPX_PLANE_Y],
                .height = img->d_h, .width = img->d_w,
                .stride = img->stride[0]
            });
        }
    }

private:
    DecCtx m_ctx;
};

Decoder::Decoder(const Gen gen)
    : ctx(std::make_unique<Ctx>(gen))
{ }

Decoder::~Decoder() = default;

void Decoder::decode(const uint8_t* packet, size_t size, const frame_handler_t& fn)
{
    ctx->decode(packet, size, fn);
}