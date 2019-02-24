#include "fresnel.hpp"

#include "spectrum.hpp"

#pragma mark - Fresnel Dielectric

struct _FresnelDielectric : FresnelDielectric {
    _FresnelDielectric(float etaI, float etaT) : m_etaI(etaI), m_etaT(etaT) {}

    Spectrum eval(float cosThetaI) const override;

    float m_etaI;
    float m_etaT;
};

Spectrum _FresnelDielectric::eval(float cosThetaI) const
{
    cosThetaI = Clamp(cosThetaI, -1.0f, 1.0f);
    // Potentially swap indices of refraction

    bool entering = cosThetaI > .0f;
    float etaI = m_etaI;
    float etaT = m_etaT;
    if (!entering) {
        std::swap(etaI, etaT);
        cosThetaI = std::abs(cosThetaI);
    }

    // Compute _cosThetaT_ using Snell's law
    float sinThetaI = std::sqrt(std::max(0.0f, 1.0f - cosThetaI * cosThetaI));
    float sinThetaT = etaI / etaT * sinThetaI;

    // Handle total internal reflection
    if (sinThetaT >= 1) {
        return { 1.f };
    }

    float cosThetaT = std::sqrt(std::max(0.0f, 1.0f - sinThetaT * sinThetaT));
    float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT))
                  / ((etaT * cosThetaI) + (etaI * cosThetaT));
    float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT))
                  / ((etaI * cosThetaI) + (etaT * cosThetaT));
    return (Rparl * Rparl + Rperp * Rperp) / 2 * Spectrum(1.f);
}

uptr<FresnelDielectric> FresnelDielectric::create(float etaI, float etaT)
{
    return std::make_unique<_FresnelDielectric>(etaI, etaT);
}

#pragma mark - Fresnel Conductor

struct _FresnelConductor : FresnelConductor {
    _FresnelConductor(const Spectrum &etaI, const Spectrum &etaT, const Spectrum &k) :
        m_etaI(etaI),
        m_etaT(etaT),
        m_k(k)
    {}

    Spectrum eval(float cosThetaI) const override;

    Spectrum m_etaI;
    Spectrum m_etaT;
    Spectrum m_k;
};

Spectrum _FresnelConductor::eval(float cosThetaI) const
{
    cosThetaI = Clamp(cosThetaI, -1.f, 1.f);
    Spectrum eta = m_etaT / m_etaI;
    Spectrum etak = m_k / m_etaI;

    float cosThetaI2 = cosThetaI * cosThetaI;
    float sinThetaI2 = 1.f - cosThetaI2;
    Spectrum eta2 = eta * eta;
    Spectrum etak2 = etak * etak;

    Spectrum t0 = eta2 - etak2 - sinThetaI2 * Spectrum(1.0f);
    Spectrum a2plusb2 = t0 * t0 + 4 * eta2 * etak2;
    Spectrum t1 = sqrt(a2plusb2) + cosThetaI2 * Spectrum(1.f);
    Spectrum a = .5f * (a2plusb2 + t0);
    Spectrum t2 = 2.f * cosThetaI * sqrt(a);
    Spectrum Rs = (t1 - t2) / (t1 + t2);

    Spectrum t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2 * Spectrum(1.f);
    Spectrum t4 = t2 * sinThetaI2;
    Spectrum Rp = Rs * (t3 - t4) / (t3 + t4);

    return 0.5f * (Rp + Rs);
}

uptr<FresnelConductor> FresnelConductor::create(const Spectrum &etaI,
                                                const Spectrum &etaT,
                                                const Spectrum &k)
{
    return std::make_unique<_FresnelConductor>(etaI, etaT, k);
}
