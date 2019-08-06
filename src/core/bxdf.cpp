#include "rt1w/bxdf.hpp"

#include "rt1w/fresnel.hpp"
#include "rt1w/interaction.hpp"
#include "rt1w/rng.hpp"
#include "rt1w/sampling.hpp"
#include "rt1w/spectrum.hpp"
#include "rt1w/utils.hpp"

#include <vector>

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

inline bool SameHemisphere(const v3f &w, const v3f &wp)
{
    return w.z * wp.z > .0f;
}

#pragma mark - BSDF

struct _BSDF : BSDF {
    _BSDF(const Interaction &i, const std::vector<sptr<BxDF>> &bxdfs) :
        m_ng(i.n),
        m_ns(i.shading.n),
        m_ss(Normalize(i.shading.dpdu)),
        m_ts(Cross(m_ns, m_ss)),
        m_bxdfs(bxdfs)
    {
        ASSERT(!HasNaN(m_ng));
        ASSERT(!HasNaN(m_ns));
        ASSERT(!HasNaN(m_ss));
        ASSERT(!HasNaN(m_ts));
    }

    Spectrum f(const v3f &woW, const v3f &wiW, BxDFType flags) const override;
    Spectrum sample_f(const v3f &woW,
                      const v2f &u,
                      v3f &wiW,
                      float &pdf,
                      BxDFType flags,
                      BxDFType *sampled) const override;
    float pdf(const v3f &woW, const v3f &wiW, BxDFType flags) const override;

    v3f localToWorld(const v3f &v) const;
    v3f worldToLocal(const v3f &v) const;

    v3f m_ng;
    v3f m_ns, m_ss, m_ts;
    std::vector<sptr<BxDF>> m_bxdfs;
};

Spectrum _BSDF::f(const v3f &woW, const v3f &wiW, BxDFType flags) const
{
    /* Convert woW and wiW from world to local coordinates */
    v3f wo = worldToLocal(woW);
    v3f wi = worldToLocal(wiW);

    /* There is a reflection if wo and wi are in the same hemisphere relative to the
     * geometric normal.
     */
    bool reflect = Dot(woW, m_ng) * Dot(wiW, m_ng) > .0f;

    Spectrum f;
    for (const auto &bxdf : m_bxdfs) {
        BxDFType type = bxdf->type();
        if (bxdf->matchesFlags(flags)
            && ((reflect && (type & BSDF_REFLECTION))
                || (!reflect && (type & BSDF_TRANSMISSION)))) {
            f += bxdf->f(wo, wi);
        }
    }
    return f;
}

Spectrum _BSDF::sample_f(const v3f &woW,
                         const v2f &u,
                         v3f &wiW,
                         float &pdf,
                         BxDFType flags,
                         BxDFType *sampled) const
{
    /* Convert woW from world to local coordinates */
    v3f wo = worldToLocal(woW);

    /* Get bxdfs matching flags */
    std::vector<sptr<BxDF>> v;
    for (const auto &bxdf : m_bxdfs) {
        if (bxdf->matchesFlags(flags)) {
            v.push_back(bxdf);
        }
    }
    if (v.empty()) {
        pdf = .0f;
        return {};
    }

    /* Randomly chose a BxDF to sample */
    size_t n = v.size();
    size_t ix = std::min((size_t)std::floor(u.x * n), n - 1);
    sptr<BxDF> bxdf = v[ix];

    /* Remap u */
    v2f uRemap = { u.x * n - ix, u.y };

    /* Sample chosen BxDF */
    v3f wi;
    Spectrum f = bxdf->sample_f(wo, uRemap, wi, pdf, sampled);
    if (FloatEqual(pdf, .0f)) {
        return {};
    }

    /* Convert wi to world coordinates */
    wiW = localToWorld(wi);
    if (!(bxdf->type() & BSDF_SPECULAR) && n > 1) {
        /* Compute pdf */
        for (const auto &other : v) {
            if (bxdf != other) {
                pdf += other->pdf(wo, wi);
            }
        }
        pdf /= n;

        /* Compute f */
        bool reflect = Dot(woW, m_ng) * Dot(wiW, m_ng) > .0f;
        for (const auto &other : m_bxdfs) {
            BxDFType type = other->type();
            if (other != bxdf && other->matchesFlags(flags)
                && ((reflect && (type & BSDF_REFLECTION))
                    || (!reflect && (type & BSDF_TRANSMISSION)))) {
                f += bxdf->f(wo, wi);
            }
        }
    }

    return f;
}

float _BSDF::pdf(const v3f &woW, const v3f &wiW, BxDFType flags) const
{
    if (m_bxdfs.empty()) {
        return .0f;
    }
    v3f wo = worldToLocal(woW);
    v3f wi = worldToLocal(wiW);

    if (FloatEqual(wo.z, .0f)) {
        return .0f;
    }

    float pdf = .0f;
    int32_t n = 0;
    for (const auto &bxdf : m_bxdfs) {
        if (bxdf->matchesFlags(flags)) {
            ++n;
            pdf += bxdf->pdf(wo, wi);
        }
    }
    return n > 0 ? pdf / n : .0f;
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
    _LambertianReflection(const Spectrum &R) :
        m_type(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)),
        m_R(R)
    {}

    BxDFType type() const override { return m_type; }
    bool matchesFlags(BxDFType flags) const override { return CheckFlags(m_type, flags); }
    Spectrum f(const v3f &wo, const v3f &wi) const override;
    Spectrum sample_f(const v3f &wo,
                      const v2f &u,
                      v3f &wi,
                      float &pdf,
                      BxDFType *sampled) const override;
    float pdf(const v3f &wo, const v3f &wi) const override;

    BxDFType m_type;
    Spectrum m_R;
};

Spectrum _LambertianReflection::f(const v3f &, const v3f &) const
{
    return m_R * (float)InvPi;
}

Spectrum _LambertianReflection::sample_f(const v3f &wo,
                                         const v2f &u,
                                         v3f &wi,
                                         float &pdf,
                                         BxDFType *sampled) const
{
    wi = CosineSampleHemisphere(u);
    if (wo.z < .0f) {
        wi *= -1.f;
    }
    pdf = this->pdf(wo, wi);
    if (sampled) {
        *sampled = m_type;
    }
    return m_R * (float)InvPi;
}

float _LambertianReflection::pdf(const v3f &wo, const v3f &wi) const
{
    return SameHemisphere(wo, wi) ? (float)(AbsCosTheta(wi) * InvPi) : .0f;
}

sptr<LambertianReflection> LambertianReflection::create(const Spectrum &R)
{
    return std::make_shared<_LambertianReflection>(R);
}

#pragma mark - Specular Reflection

struct _SpecularReflection : SpecularReflection {
    _SpecularReflection(const Spectrum &R, uptr<Fresnel> fresnel) :
        m_type(BxDFType(BSDF_REFLECTION | BSDF_SPECULAR)),
        m_R(R),
        m_fresnel(std::move(fresnel))
    {}

    BxDFType type() const override { return m_type; }
    bool matchesFlags(BxDFType flags) const override { return CheckFlags(m_type, flags); }
    Spectrum f(const v3f &, const v3f &) const override { return {}; }
    Spectrum sample_f(const v3f &wo,
                      const v2f &u,
                      v3f &wi,
                      float &pdf,
                      BxDFType *sampled) const override;
    float pdf(const v3f &, const v3f &) const override { return .0f; }

    BxDFType m_type;
    Spectrum m_R;
    uptr<Fresnel> m_fresnel;
};

Spectrum _SpecularReflection::sample_f(const v3f &wo,
                                       const v2f &,
                                       v3f &wi,
                                       float &pdf,
                                       BxDFType *sampled) const
{
    wi = v3f{ -wo.x, -wo.y, wo.z };
    pdf = 1.f;
    if (sampled) {
        *sampled = m_type;
    }
    return m_fresnel->eval(CosTheta(wi)) * m_R / AbsCosTheta(wi);
}

sptr<SpecularReflection> SpecularReflection::create(const Spectrum &R,
                                                    uptr<Fresnel> fresnel)
{
    return std::make_shared<_SpecularReflection>(R, std::move(fresnel));
}

#pragma mark - Specular Transmission

struct _SpecularTransmission : SpecularTransmission {
    _SpecularTransmission(const Spectrum &T, float etaA, float etaB) :
        m_type(BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR)),
        m_T(T),
        m_etaA(etaA),
        m_etaB(etaB),
        m_fresnel(FresnelDielectric::create(etaA, etaB))
    {}

    BxDFType type() const override { return m_type; }
    bool matchesFlags(BxDFType flags) const override { return CheckFlags(m_type, flags); }
    Spectrum f(const v3f &, const v3f &) const override { return {}; }
    Spectrum sample_f(const v3f &wo,
                      const v2f &u,
                      v3f &wi,
                      float &pdf,
                      BxDFType *sampled) const override;
    float pdf(const v3f &, const v3f &) const override { return .0f; }

    BxDFType m_type;
    Spectrum m_T;
    float m_etaA;
    float m_etaB;
    uptr<Fresnel> m_fresnel;
};

Spectrum _SpecularTransmission::sample_f(const v3f &wo,
                                         const v2f &,
                                         v3f &wi,
                                         float &pdf,
                                         BxDFType *sampled) const
{
    bool entering = CosTheta(wo) > 0;
    float etaI = entering ? m_etaA : m_etaB;
    float etaT = entering ? m_etaB : m_etaA;

    v3f n = FaceForward({ 0.0f, 0.0f, 1.0f }, wo);
    if (Refract(wo, n, etaI / etaT, wi)) {
        Spectrum ft = m_T * (Spectrum(1.f) - m_fresnel->eval(CosTheta(wi)));
        pdf = 1.f;
        if (sampled) {
            *sampled = m_type;
        }
        return ft / AbsCosTheta(wi);
    }
    pdf = .0f;
    return {};
}

sptr<SpecularTransmission> SpecularTransmission::create(const Spectrum &T,
                                                        float etaA,
                                                        float etaB)
{
    return std::make_shared<_SpecularTransmission>(T, etaA, etaB);
}

#pragma mark - Fresnel Specular

struct _FresnelSpecular : FresnelSpecular {
    _FresnelSpecular(const Spectrum &R, const Spectrum &T, float etaA, float etaB) :
        m_type(BxDFType(BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_SPECULAR)),
        m_R(R),
        m_T(T),
        m_etaA(etaA),
        m_etaB(etaB),
        m_fresnel(FresnelDielectric::create(etaA, etaB))
    {}

    BxDFType type() const override { return m_type; }
    bool matchesFlags(BxDFType flags) const override { return CheckFlags(m_type, flags); }
    Spectrum f(const v3f &, const v3f &) const override { return {}; }
    Spectrum sample_f(const v3f &wo,
                      const v2f &u,
                      v3f &wi,
                      float &pdf,
                      BxDFType *sampled) const override;
    float pdf(const v3f &, const v3f &) const override { return .0f; }

    BxDFType m_type;
    Spectrum m_R;
    Spectrum m_T;
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

Spectrum _FresnelSpecular::sample_f(const v3f &wo,
                                    const v2f &u,
                                    v3f &wi,
                                    float &pdf,
                                    BxDFType *sampled) const
{
    bool entering = CosTheta(wo) > 0;
    float etaI = entering ? m_etaA : m_etaB;
    float etaT = entering ? m_etaB : m_etaA;

    v3f n = FaceForward({ 0.0f, 0.0f, 1.0f }, wo);

    bool refract = Refract(wo, n, etaI / etaT, wi);
    Spectrum F = m_fresnel->eval(CosTheta(wi));
    Spectrum ft = m_R * F;
    if (refract) {
        ft += m_T * (Spectrum(1.f) - m_fresnel->eval(CosTheta(wi)));
        if (u.x * u.y < schlick(CosTheta(wi), etaI)) {
            wi = Reflect(wo, n);
        }
    }
    pdf = 1.f;
    if (sampled) {
        *sampled = m_type;
    }
    return ft / AbsCosTheta(wi);
}

sptr<FresnelSpecular> FresnelSpecular::create(const Spectrum &R,
                                              const Spectrum &T,
                                              float etaA,
                                              float etaB)
{
    return std::make_shared<_FresnelSpecular>(R, T, etaA, etaB);
}
