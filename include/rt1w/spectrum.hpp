#pragma once

#include "geometry.hpp"
#include "utils.hpp"

template <size_t N>
struct SampledSpectrum {
    SampledSpectrum() = default;
    SampledSpectrum(float v)
    {
        for (size_t i = 0; i < N; ++i) {
            m_c[i] = v;
        }
    }

    bool isBlack() const;
    v3f rgb() const { return { m_c[0], m_c[1], m_c[2] }; }

    SampledSpectrum operator+(const SampledSpectrum &s) const;
    SampledSpectrum &operator+=(const SampledSpectrum &s);
    SampledSpectrum operator-(const SampledSpectrum &s) const;
    SampledSpectrum operator*(float f) const;
    SampledSpectrum operator*=(float f);
    SampledSpectrum operator*(const SampledSpectrum &s) const;
    SampledSpectrum &operator*=(const SampledSpectrum &s);
    SampledSpectrum operator/(float f) const;
    SampledSpectrum &operator/=(float f);
    SampledSpectrum operator/(const SampledSpectrum &s) const;
    float operator[](size_t i) const;
    float &operator[](size_t i);

    float m_c[N] = {};
};

template <size_t N>
inline bool SampledSpectrum<N>::isBlack() const
{
    for (size_t i = 0; i < N; ++i) {
        if (!FloatEqual(m_c[i], .0f)) {
            return false;
        }
    }
    return true;
}
template <size_t N>
inline SampledSpectrum<N> SampledSpectrum<N>::operator+(const SampledSpectrum<N> &s) const
{
    auto rs = *this;
    for (size_t i = 0; i < N; ++i) {
        rs[i] += s[i];
    }
    return rs;
}

template <size_t N>
inline SampledSpectrum<N> &SampledSpectrum<N>::operator+=(const SampledSpectrum<N> &s)
{
    for (size_t i = 0; i < N; ++i) {
        this->m_c[i] += s[i];
    }
    return *this;
}

template <size_t N>
inline SampledSpectrum<N> SampledSpectrum<N>::operator-(const SampledSpectrum<N> &s) const
{
    auto rs = *this;
    for (size_t i = 0; i < N; ++i) {
        rs[i] -= s[i];
    }
    return rs;
}

template <size_t N>
inline SampledSpectrum<N> SampledSpectrum<N>::operator*(float f) const
{
    auto rs = *this;
    for (size_t i = 0; i < N; ++i) {
        rs[i] *= f;
    }
    return rs;
}

template <size_t N>
inline SampledSpectrum<N> SampledSpectrum<N>::operator*=(float f)
{
    for (size_t i = 0; i < N; ++i) {
        this->m_c[i] *= f;
    }
    return *this;
}

template <size_t N>
inline SampledSpectrum<N> SampledSpectrum<N>::operator*(const SampledSpectrum<N> &s) const
{
    auto rs = *this;
    for (size_t i = 0; i < N; ++i) {
        rs.m_c[i] *= s[i];
    }
    return rs;
}

template <size_t N>
inline SampledSpectrum<N> &SampledSpectrum<N>::operator*=(const SampledSpectrum<N> &s)
{
    for (size_t i = 0; i < N; ++i) {
        this->m_c[i] *= s[i];
    }
    return *this;
}

template <size_t N>
inline SampledSpectrum<N> operator*(float f, const SampledSpectrum<N> &s)
{
    return { s * f };
}

template <size_t N>
inline SampledSpectrum<N> SampledSpectrum<N>::operator/(float f) const
{
    auto rs = *this;
    for (size_t i = 0; i < N; ++i) {
        rs.m_c[i] /= f;
    }
    return rs;
}

template <size_t N>
inline SampledSpectrum<N> &SampledSpectrum<N>::operator/=(float f)
{
    for (size_t i = 0; i < N; ++i) {
        this->m_c[i] *= f;
    }
    return *this;
}

template <size_t N>
inline SampledSpectrum<N> SampledSpectrum<N>::operator/(const SampledSpectrum<N> &s) const
{
    auto rs = *this;
    for (size_t i = 0; i < N; ++i) {
        rs.m_c[i] /= s[i];
    }
    return rs;
}

template <size_t N>
inline float SampledSpectrum<N>::operator[](size_t i) const
{
    ASSERT(i < N);
    return m_c[i];
}

template <size_t N>
inline float &SampledSpectrum<N>::operator[](size_t i)
{
    ASSERT(i < N);
    return m_c[i];
}

template <size_t N>
inline float MaxComponent(const SampledSpectrum<N> &s)
{
    float min = s.m_c[0];
    for (size_t i = 1; i < N; ++i) {
        if (min > s.m_c[i]) {
            min = s.m_c[i];
        }
    }
    return min;
}

template <size_t N>
inline SampledSpectrum<N> sqrt(const SampledSpectrum<N> &s)
{
    SampledSpectrum<N> rs;
    for (size_t i = 0; i < N; ++i) {
        rs.m_c[i] = std::sqrt(s[i]);
    }
    return rs;
}

struct Spectrum : SampledSpectrum<3> {
    static Spectrum fromRGB(const v3f &v)
    {
        Spectrum s;
        memcpy(s.m_c, &v.x, sizeof(v));
        return s;
    }

    Spectrum() = default;
    Spectrum(float v) : SampledSpectrum<3>(v) {}
    Spectrum(const SampledSpectrum<3> &v) : SampledSpectrum(v) {}
};
