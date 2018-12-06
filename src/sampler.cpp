#include "sampler.hpp"

#include "error.h"
#include "rng.hpp"

#include <algorithm>
#include <vector>

static void GenerateSamples1D(float *smp, size_t n, RNG &rng, bool jitter)
{
    float inv_n = 1.0f / n;

    for (size_t i = 0; i < n; i++) {
        float d = jitter ? rng.f32() : 0.5f;
        smp[i] = std::min((i + d) * inv_n, OneMinusEpsilon_f32);
    }
}

static void GenerateSamples2D(v2f *smp, size_t nx, size_t ny, RNG &rng, bool jitter)
{
    float dx = 1.0f / nx;
    float dy = 1.0f / ny;

    size_t i = 0;
    for (size_t x = 0; x < nx; x++) {
        for (size_t y = 0; y < ny; y++) {
            float jx = jitter ? rng.f32() : 0.5f;
            float jy = jitter ? rng.f32() : 0.5f;

            smp[i].x = std::min((x + jx) * dx, OneMinusEpsilon_f32);
            smp[i].y = std::min((y + jy) * dy, OneMinusEpsilon_f32);
            i++;
        }
    }
}

template <typename T>
static void Shuffle(T *smp, size_t n, RNG &rng)
{
    for (size_t i = 0; i < n - 1; i++) {
        size_t j = i + rng.u32((uint32_t)(n - i));
        std::swap(smp[i], smp[j]);
    }
}

struct _Sampler : Sampler {
    _Sampler(uint32_t x, uint32_t y, uint32_t dim, bool jitter) :
        m_spp(x * y),
        m_x(x),
        m_y(y),
        m_dim(dim),
        m_jitter(jitter),
        m_rng(RNG::create())
    {}
    void init();

    uint64_t samplesPerPixel() const override { return m_spp; }
    sptr<Sampler> clone() const override;

    float sample1D() override;
    v2f sample2D() override;
    CameraSample cameraSample() override;

    void startPixel(v2i p) override;
    bool startNextSample() override;

    const uint64_t m_spp;
    const uint32_t m_x;
    const uint32_t m_y;
    const uint32_t m_dim;
    const bool m_jitter;

    v2i m_pixel;    // Current pixel
    size_t m_ix;    // Current sample

    std::vector<std::vector<float>> m_samples1D;
    std::vector<std::vector<v2f>> m_samples2D;
    size_t m_1d_dim;
    size_t m_2d_dim;

    uptr<RNG> m_rng;
};

void _Sampler::init()
{
    for (size_t i = 0; i < m_spp; i++) {
        m_samples1D.push_back(std::vector<float>(m_spp));
        m_samples2D.push_back(std::vector<v2f>(m_spp));
    }
}

sptr<Sampler> _Sampler::clone() const
{
    return Sampler::create(m_x, m_y, m_dim, m_jitter);
}

float _Sampler::sample1D()
{
    if (m_1d_dim < m_samples1D.size()) {
        return m_samples1D[m_1d_dim++][m_ix];
    }
    return m_rng->f32();
}

v2f _Sampler::sample2D()
{
    if (m_2d_dim < m_samples2D.size()) {
        return m_samples2D[m_2d_dim++][m_ix];
    }
    return { m_rng->f32(), m_rng->f32() };
}

CameraSample _Sampler::cameraSample()
{
    v2f p = v2f{ (float)m_pixel.x, (float)m_pixel.y };
    return { p + sample2D(), sample2D() };
}

void _Sampler::startPixel(v2i p)
{
    m_pixel = p;
    m_ix = 0;
    m_1d_dim = 0;
    m_2d_dim = 0;

    for (size_t i = 0; i < m_samples1D.size(); i++) {
        GenerateSamples1D(&m_samples1D[i][0], (size_t)m_spp, *m_rng, m_jitter);
        Shuffle(&m_samples1D[i][0], m_spp, *m_rng);
    }
    for (size_t i = 0; i < m_samples2D.size(); i++) {
        GenerateSamples2D(&m_samples2D[i][0], m_x, m_y, *m_rng, m_jitter);
        Shuffle(&m_samples2D[i][0], m_spp, *m_rng);
    }
}

bool _Sampler::startNextSample()
{
    if (++m_ix < m_spp) {
        m_1d_dim = 0;
        m_2d_dim = 0;
        return true;
    }
    return false;
}

#pragma mark - Static Constructors

sptr<Sampler> Sampler::create(uint32_t x, uint32_t y, uint32_t dim, bool jitter)
{
    auto sampler = std::make_shared<_Sampler>(x, y, dim, jitter);
    sampler->init();
    return sampler;
}
