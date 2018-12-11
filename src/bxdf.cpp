#include "bxdf.hpp"

#include "fresnel.hpp"
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

    /* There is a reflection if wo and wi are in the same hemisphere relative to the
     * geometric normal
     */
    bool reflect = Dot(woW, m_ng) * Dot(wiW, m_ng) > 0;

    v3f f = {};
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

#pragma mark - Specular Reflection

struct _SpecularReflection : SpecularReflection {
    _SpecularReflection(const v3f &R, uptr<Fresnel> fresnel) :
        m_type(BxDFType(BSDF_REFLECTION | BSDF_SPECULAR)),
        m_R(R),
        m_fresnel(std::move(fresnel))
    {}

    BxDFType type() const override { return m_type; }
    bool matchesFlags(BxDFType flags) const override { return CheckFlags(m_type, flags); }
    v3f f(const v3f &, const v3f &) const override { return {}; }
    v3f sample_f(const v3f &wo, v3f &wi) const override;

    BxDFType m_type;
    v3f m_R;
    uptr<Fresnel> m_fresnel;
};

v3f _SpecularReflection::sample_f(const v3f &wo, v3f &wi) const
{
    wi = v3f{ -wo.x, -wo.y, wo.z };
    return m_R;

    return m_fresnel->eval(CosTheta(wi)) * m_R / AbsCosTheta(wi);
}

sptr<SpecularReflection> SpecularReflection::create(const v3f &R, uptr<Fresnel> fresnel)
{
    return std::make_shared<_SpecularReflection>(R, std::move(fresnel));
}

#pragma mark - Specular Transmission

struct _SpecularTransmission : SpecularTransmission {
    _SpecularTransmission(const v3f &T, float etaA, float etaB) :
        m_type(BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR)),
        m_T(T),
        m_etaA(etaA),
        m_etaB(etaB),
        m_fresnel(FresnelDielectric::create(etaA, etaB))
    {}

    BxDFType type() const override { return m_type; }
    bool matchesFlags(BxDFType flags) const override { return CheckFlags(m_type, flags); }
    v3f f(const v3f &, const v3f &) const override { return {}; }
    v3f sample_f(const v3f &wo, v3f &wi) const override;

    BxDFType m_type;
    v3f m_T;
    float m_etaA;
    float m_etaB;
    uptr<Fresnel> m_fresnel;
};

v3f _SpecularTransmission::sample_f(const v3f &wo, v3f &wi) const
{
    bool entering = CosTheta(wo) > 0;
    float etaI = entering ? m_etaA : m_etaB;
    float etaT = entering ? m_etaB : m_etaA;

    v3f n = FaceForward({ 0.0f, 0.0f, 1.0f }, wo);
    if (Refract(wo, n, etaI / etaT, wi)) {
        v3f ft = m_T * (v3f{ 1.0, 1.0, 1.0 } - m_fresnel->eval(CosTheta(wi)));

        return ft / AbsCosTheta(wi);
    }
    return {};
}

sptr<SpecularTransmission> SpecularTransmission::create(const v3f &T,
                                                        float etaA,
                                                        float etaB)
{
    return std::make_shared<_SpecularTransmission>(T, etaA, etaB);
}

#pragma mark - Fresnel Specular

struct _FresnelSpecular : FresnelSpecular {
    _FresnelSpecular(const v3f &R, const v3f &T, float etaA, float etaB) :
        m_type(BxDFType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_SPECULAR)),
        m_R(R),
        m_T(T),
        m_etaA(etaA),
        m_etaB(etaB),
        m_fresnel(FresnelDielectric::create(etaA, etaB))
    {}

    BxDFType type() const override { return m_type; }
    bool matchesFlags(BxDFType flags) const override { return CheckFlags(m_type, flags); }
    v3f f(const v3f &, const v3f &) const override { return {}; }
    v3f sample_f(const v3f &wo, v3f &wi) const override;

    BxDFType m_type;
    v3f m_R;
    v3f m_T;
    float m_etaA;
    float m_etaB;
    uptr<Fresnel> m_fresnel;
};

static float schlick(float cos, float ri)
{
    float r = (1.0f - ri) / (1.0f + ri);
    r = r * r;
    return r + (1.0f - r) * powf((1.0f - cos), 5.0f);
}

v3f _FresnelSpecular::sample_f(const v3f &wo, v3f &wi) const
{
    bool entering = CosTheta(wo) > 0;
    float etaI = entering ? m_etaA : m_etaB;
    float etaT = entering ? m_etaB : m_etaA;

    v3f n = FaceForward({ 0.0f, 0.0f, 1.0f }, wo);

    bool refract = Refract(wo, n, etaI / etaT, wi);
    v3f F = m_fresnel->eval(CosTheta(wi));
    v3f ft = m_R * F;
    if (refract) {
        static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        ft += m_T * (v3f{ 1.0, 1.0, 1.0 } - m_fresnel->eval(CosTheta(wi)));
        if (dist(__prng) < schlick(CosTheta(wi), etaI)) {
            wi = Reflect(wo, n);
        }
    }
    return ft / AbsCosTheta(wi);
}

sptr<FresnelSpecular> FresnelSpecular::create(const v3f &R,
                                              const v3f &T,
                                              float etaA,
                                              float etaB)
{
    return std::make_shared<_FresnelSpecular>(R, T, etaA, etaB);
}
