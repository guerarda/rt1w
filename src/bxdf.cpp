#include "bxdf.hpp"

#include "interaction.hpp"
#include "rng.hpp"
#include "utils.hpp"

#include <random>

static std::random_device rd;
static std::mt19937 __prng(rd());

static v3f random_sphere_point()
{
    static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    v3f p;

    do {
        p.x = dist(__prng);
        p.y = dist(__prng);
        p.z = dist(__prng);
    } while (p.length_sq() >= 1.0f);

    return p;
}

bool Refract(const v3f &wi, const v3f &n, float eta, v3f &wt)
{
    float cosThetaI = Dot(wi, n);
    float sin2ThetaI = std::max(0.0f, 1.0f - cosThetaI * cosThetaI);
    float sin2ThetaT = eta * eta * sin2ThetaI;

    if (sin2ThetaT >= 1) {
        return false;
    }
    float cosThetaT = std::sqrt(1 - sin2ThetaT);

    wt = -eta * wi + (eta * cosThetaI - cosThetaT) * n;
    return true;
}

#pragma mark - BSDF

struct _BSDF : BSDF {
    _BSDF(const Interaction &i, const std::vector<sptr<BxDF>> &bxdfs) :
        m_ng(i.n),
        m_ns(i.shading.n),
        m_ss(Normalize(i.shading.dpdu)),
        m_ts(Cross(m_ns, m_ts)),
        m_bxdfs(bxdfs),
        m_rng(RNG::create())
    {}

    v3f f(const v3f &woW, const v3f &wiW) const override;
    v3f sample_f(const v3f &woW, v3f &wiW) const override;

    v3f localToWorld(const v3f &v) const;
    v3f worldToLocal(const v3f &v) const;

    v3f m_ng;
    v3f m_ns, m_ss, m_ts;
    std::vector<sptr<BxDF>> m_bxdfs;
    uptr<RNG> m_rng;
};

v3f _BSDF::f(const v3f &woW, const v3f &wiW) const
{
    /* Convert woW and wiW from world to local coordinates */
    v3f wo = worldToLocal(woW);
    v3f wi = worldToLocal(wiW);

    v3f f = {};

    /* There is a reflection if wo and wi are in the same hemisphere relative to the
     * geometric normal
     */
    bool reflect = Dot(woW, m_ng) * Dot(wiW, m_ng) > 0;
    for (auto &bxdf : m_bxdfs) {
        BxDFType type = bxdf->type();
        if ((reflect && (type & BSDF_REFLECTION))
            || (!reflect && (type & BSDF_TRANSMISSION))) {
            f += bxdf->f(wo, wi);
        }
    }
    return f;
}

v3f _BSDF::sample_f(const v3f &woW, v3f &wiW) const
{
    /* Convert woW from world to local coordinates */
    v3f wo = worldToLocal(woW);

    /* Randomly chose a BxDF to sample */
    size_t ix = m_rng->u32((uint32_t)m_bxdfs.size());

    v3f wi;
    v3f f = m_bxdfs[ix]->sample_f(wo, wi);

    /* Convert wi to world coordinates */
    wiW = localToWorld(wi);

    return f;
}

v3f _BSDF::localToWorld(const v3f &v) const
{
    return {
        m_ss.x * v.x + m_ts.x * v.y + m_ns.x * v.z,
        m_ss.y * v.x + m_ts.y * v.y + m_ns.y * v.z,
        m_ss.z * v.x + m_ts.z * v.y + m_ns.z * v.z,
    };
}

v3f _BSDF::worldToLocal(const v3f &v) const
{
    return { Dot(v, m_ss), Dot(v, m_ts), Dot(v, m_ns) };
}

sptr<BSDF> BSDF::create(const Interaction &i, const std::vector<sptr<BxDF>> &bxdfs)
{
    return std::make_shared<_BSDF>(i, bxdfs);
}

#pragma mark - BxDF

static inline bool CheckFlags(BxDFType type, BxDFType flags)
{
    return (type & flags) == type;
}
#pragma mark - Lambertian Reflection

struct _LambertianReflection : LambertianReflection {
    _LambertianReflection(v3f R) :
        m_type(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)),
        m_R(R)
    {}

    BxDFType type() const override { return m_type; }
    bool matchesFlags(BxDFType flags) const override { return CheckFlags(m_type, flags); }
    v3f f(const v3f &wo, const v3f &wi) const override;
    v3f sample_f(const v3f &wo, v3f &wi) const override;

    BxDFType m_type;
    v3f m_R;
};

v3f _LambertianReflection::f(const v3f &, const v3f &) const
{
    return m_R / (float)Pi;
}

v3f _LambertianReflection::sample_f(const v3f &, v3f &wi) const
{
    wi = Normalize(random_sphere_point());

    return m_R / (float)Pi;
}

sptr<LambertianReflection> LambertianReflection::create(v3f R)
{
    return std::make_shared<_LambertianReflection>(R);
}


