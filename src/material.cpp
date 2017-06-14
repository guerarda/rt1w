#include "material.hpp"
#include <random>
#include <math.h>

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

static bool refract(const v3f &v, const v3f &n, float ni_over_nt, v3f &refract)
{
    v3f uv = v.normalized();
    float dt = Dot(uv, n);
    float delta = 1.0f - ni_over_nt * ni_over_nt * ( 1.0f - dt * dt);

    if (delta > 0.0f) {
        refract = ni_over_nt * (uv - dt * n);
        refract = refract - sqrtf(delta) * n;
        return true;
    } else {
        return false;
    }
}

static float schlick(float cos, float ri)
{
    float r = (1.0f - ri) / (1.0f + ri);
    r  = r * r;
    return r + (1.0f - r) * powf((1.0f - cos), 5.0f);
}

#pragma mark - Lambertian

struct _Lambertian : Lambertian {

    _Lambertian(const sptr<Texture> &tex) : m_albedo(tex) { }

    bool scatter(const sptr<ray> &r_in,
                 const hit_record &rec,
                 v3f &attenuation,
                 sptr<ray> &scattered) const;

    sptr<Texture>  m_albedo;
};

bool _Lambertian::scatter(__unused const sptr<ray> &r_in,
                          const hit_record &rec,
                          v3f &attenuation,
                          sptr<ray> &scattered) const
{
    v3f target = rec.p + rec.normal + random_sphere_point();
    scattered = ray::create(rec.p, target - rec.p);
    attenuation = m_albedo->value(rec.uv.x, rec.uv.y, rec.p);
    return true;
}

#pragma mark - Metal

struct _Metal : Metal {

    _Metal(const sptr<Texture> &tex, float f);

    bool scatter(const sptr<ray> &r_in,
                 const hit_record &rec,
                 v3f &attenuation,
                 sptr<ray> &scattered) const;

    sptr<Texture> m_albedo;
    float         m_fuzz;
};

_Metal::_Metal(const sptr<Texture> &tex, float f)
{
    m_albedo = tex;
    m_fuzz = fminf(1.0f, f);
}

bool _Metal::scatter(const sptr<ray> &r_in,
                     const hit_record &rec,
                     v3f &attenuation,
                     sptr<ray> &scattered) const
{
    v3f reflected = Reflect(r_in->direction(), rec.normal);
    v3f fuzz = m_fuzz * random_sphere_point();
    scattered = ray::create(rec.p, fuzz + reflected);
    attenuation = m_albedo->value(rec.uv.x, rec.uv.y, rec.p);
    return Dot(scattered->direction(), rec.normal) > 0.0f;
}

#pragma mark - Dieletric

struct _Dielectric : Dielectric {

    _Dielectric(float ri) : m_ref_idx(ri) { }

    bool scatter(const sptr<ray> &r_in,
                 const hit_record &rec,
                 v3f &attenuation,
                 sptr<ray> &scattered) const;

    float m_ref_idx;
};

bool _Dielectric::scatter(const sptr<ray> &r_in,
                          const hit_record &rec,
                          v3f &attenuation,
                          sptr<ray> &scattered) const
{
    float ni_over_nt;
    float cosine;
    float p_reflected;
    v3f norm_out;
    v3f refracted;
    v3f rdir = r_in->direction();

    attenuation = { 1.0f, 1.0f, 1.0f };
    if (Dot(r_in->direction(), rec.normal) > 0.0f) {
        norm_out = -rec.normal;
        ni_over_nt = m_ref_idx;
        cosine = m_ref_idx * Dot(rdir, rec.normal) / rdir.length();
    } else {
        norm_out = rec.normal;
        ni_over_nt = 1.0f / m_ref_idx;
        cosine = -Dot(rdir, rec.normal) / rdir.length();
    }
    if (refract(r_in->direction(), norm_out, ni_over_nt, refracted)) {
        p_reflected = schlick(cosine, m_ref_idx);
    } else {
        p_reflected = 1.0f;
    }
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    if (dist(__prng) < p_reflected) {
        v3f reflected = Reflect(rdir, rec.normal);
        scattered = ray::create(rec.p, reflected);
    } else {
        scattered = ray::create(rec.p, refracted);
    }
    return true;
}

#pragma mark - Static constructors

sptr<Lambertian> Lambertian::create(const sptr<Texture> &tex)
{
    return std::make_shared<_Lambertian>(tex);
}

sptr<Metal> Metal::create(const sptr<Texture> &tex, float f)
{
    return std::make_shared<_Metal>(tex, f);
}

sptr<Dielectric> Dielectric::create(float ri)
{
    return std::make_shared<_Dielectric>(ri);
}
