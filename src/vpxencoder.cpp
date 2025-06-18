#include "vpxencoder.hpp"
#include "vpxcommon_impl.hpp"
#include <algorithm>
#include <vpx/vpx_encoder.h>
#include <vpx/vp8cx.h>
#include <iostream>

using namespace Vpx;

namespace {

constexpr vpx_codec_iface_t* encoder_vp8 = &vpx_codec_vp8_cx_algo;
constexpr vpx_codec_iface_t* encoder_vp9 = &vpx_codec_vp9_cx_algo;

struct EncCtx
{
    EncCtx(const Encoder::Config& config)
    {
        // Encoder config from defaults + provided settings
        vpx_codec_enc_cfg_t cfg;
        const auto* encoder_iface = (config.gen == Gen::Vp8) ? encoder_vp8 : encoder_vp9;
        if (vpx_codec_enc_config_default(encoder_iface, &cfg, 0) != VPX_CODEC_OK) {
            throw std::runtime_error("Failed to get default codec config");
        }
        cfg.g_w = config.width;
        cfg.g_h = config.height;
        cfg.g_timebase.num = 1;
        cfg.g_timebase.den = config.fps;
        cfg.rc_target_bitrate = config.bitrate;
        cfg.g_error_resilient = 0;
        cfg.g_threads = config.threads;
        // Create encoder context
        constexpr auto use_flags = 0;
        if (vpx_codec_enc_init(&m_ctx, encoder_iface, &cfg, use_flags) != VPX_CODEC_OK) {
            throw std::runtime_error("Failed to create encoding context");
        }
        // Set codec parameters (destroy ctx on fail)
        try {
            if (vpx_codec_control(&m_ctx, VP8E_SET_CPUUSED, config.cpu_used) != VPX_CODEC_OK) {
                throw std::runtime_error("Failed to set CPUUSED parameter");
            }
        }
        catch(...) {
            vpx_codec_destroy(&m_ctx);
            throw;
        }
    }

    EncCtx(const EncCtx&) = delete;
    void operator=(const EncCtx&) = delete;

    ~EncCtx() noexcept { vpx_codec_destroy(&m_ctx); }

    constexpr operator vpx_codec_ctx_t*() noexcept { return &m_ctx; }
    constexpr operator const vpx_codec_ctx_t*() const noexcept { return &m_ctx; }

private:
    vpx_codec_ctx_t m_ctx;
};

}  // private namespace


struct Encoder::Ctx {
    Ctx(const Encoder::Config& config)
        : m_img(config.width, config.height)
        , m_ctx(config)
    { }

    constexpr Vpx::Image& frame() noexcept { return m_img; }
    constexpr const Vpx::Image& frame() const noexcept { return m_img; }

    constexpr const auto frameIdx() const noexcept { return m_frameIdx; }

    template<typename Fn> void encode(Fn&& fn)
    {
        constexpr vpx_enc_frame_flags_t flags = 0;
        if (vpx_codec_encode(m_ctx, m_img, ++m_frameIdx, 1, flags, VPX_DL_REALTIME) != VPX_CODEC_OK) {
            throw std::runtime_error("Failed to encode frame");
        }

        vpx_codec_iter_t iter = nullptr;
        const vpx_codec_cx_pkt_t *pkt = nullptr;
        while ((pkt = vpx_codec_get_cx_data(m_ctx, &iter)) != nullptr) {
            if (pkt->kind == VPX_CODEC_CX_FRAME_PKT) {
                fn(static_cast<const uint8_t*>(pkt->data.raw.buf), pkt->data.raw.sz);
            }
        }
    }

private:
    unsigned int m_frameIdx = 0;
    Vpx::Image m_img;
    EncCtx m_ctx;
};

Encoder::Encoder(const Config& config)
    : ctx(std::make_unique<Ctx>(config))
{ }

Encoder::~Encoder() = default;

void Encoder::encode(const packet_handler_t& fn)
{
    ctx->encode(fn);
}

Plane Encoder::yPlane() const noexcept
{
    return ctx->frame().yPlane();
}

void Encoder::copyFromGray(const uint8_t* data)
{
    ctx->frame().copyFromGrayscale(data);
}