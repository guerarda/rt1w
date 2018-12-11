#include "fresnel.hpp"

#pragma mark - Fresnel Dielectric

struct _FresnelDielectric : FresnelDielectric {
    _FresnelDielectric(float etaI, float etaT) : m_etaI(etaI), m_etaT(etaT) {}

    v3f eval(float cosThetaI) const override;

    float m_etaI;
    float m_etaT;
};

v3f _FresnelDielectric::eval(float cosThetaI) const
{
    cosThetaI = Clamp(cosThetaI, -1.0f, 1.0f);
    // Potentially swap indices of refraction

    bool entering = cosThetaI > 0.f;
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
        return { 1.0f, 1.0f, 1.0f };
    }

    float cosThetaT = std::sqrt(std::max(0.0f, 1.0f - sinThetaT * sinThetaT));
    float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT))
                  / ((etaT * cosThetaI) + (etaI * cosThetaT));
    float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT))
                  / ((etaI * cosThetaI) + (etaT * cosThetaT));
    return (Rparl * Rparl + Rperp * Rperp) / 2 * v3f{ 1.0f, 1.0f, 1.0f };
}

uptr<FresnelDielectric> FresnelDielectric::create(float etaI, float etaT)
{
    return std::make_unique<_FresnelDielectric>(etaI, etaT);
}

#pragma mark - Fresnel Conductor

struct _FresnelConductor : FresnelConductor {
    _FresnelConductor(v3f etaI, v3f etaT, v3f k) : m_etaI(etaI), m_etaT(etaT), m_k(k) {}

    v3f eval(float cosThetaI) const override;

    v3f m_etaI;
    v3f m_etaT;
    v3f m_k;
};

v3f _FresnelConductor::eval(float cosThetaI) const
{
    cosThetaI = Clamp(cosThetaI, -1, 1);
    v3f eta = { m_etaT.x / m_etaI.x, m_etaT.y / m_etaI.y, m_etaT.z / m_etaI.z };
    v3f etak = { m_k.x / m_etaI.x, m_k.y / m_etaI.y, m_k.z / m_etaI.z };

    float cosThetaI2 = cosThetaI * cosThetaI;
    float sinThetaI2 = 1.0f - cosThetaI2;
    v3f eta2 = eta * eta;
    v3f etak2 = etak * etak;

    v3f t0 = eta2 - etak2 - sinThetaI2 * v3f{ 1.0f, 1.0f, 1.0f };
    v3f a2plusb2 = t0 * t0 + 4 * eta2 * etak2;
    a2plusb2 = { std::sqrt(a2plusb2.x), std::sqrt(a2plusb2.y), std::sqrt(a2plusb2.z) };
    v3f t1 = a2plusb2 + cosThetaI2 * v3f{ 1.0f, 1.0f, 1.0f };
    v3f a = 0.5f * (a2plusb2 + t0);
    a = { std::sqrt(a.x), std::sqrt(a.y), std::sqrt(a.z) };
    v3f t2 = (float)2 * cosThetaI * a;
    v3f Rs = (t1 - t2) / (t1 + t2);

    v3f t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2 * v3f{ 1.0f, 1.0f, 1.0f };
    v3f t4 = t2 * sinThetaI2;
    v3f Rp = Rs * (t3 - t4) / (t3 + t4);

    return 0.5f * (Rp + Rs);
}

uptr<FresnelConductor> FresnelConductor::create(v3f etaI, v3f etaT, v3f k)
{
    return std::make_unique<_FresnelConductor>(etaI, etaT, k);
}
