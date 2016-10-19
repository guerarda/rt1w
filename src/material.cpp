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
    } while (v3f_norm_sq(p) >= 1.0f);

    return p;
}

static bool refract(const v3f &v, const v3f &n, float ni_over_nt, v3f &refract)
{
    v3f uv = v3f_normalize(v);
    float dt = v3f_dot(uv, n);
    float delta = 1.0f - ni_over_nt * ni_over_nt * ( 1.0f - dt * dt);

    if (delta > 0.0f) {
        refract = v3f_smul(ni_over_nt, v3f_sub(uv, v3f_smul(dt, n)));
        refract = v3f_sub(refract, v3f_smul(sqrtf(delta), n));
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

struct _lambertian : lambertian {

    _lambertian(const sptr<Texture> &tex) : m_albedo(tex) { }

    bool scatter(const sptr<ray> &r_in,
                 const hit_record &rec,
                 v3f &attenuation,
                 sptr<ray> &scattered) const;

    sptr<Texture>  m_albedo;
};

bool _lambertian::scatter(__unused const sptr<ray> &r_in,
                          const hit_record &rec,
                          v3f &attenuation,
                          sptr<ray> &scattered) const
{
    v3f target = v3f_add(v3f_add(rec.p, rec.normal),
                         random_sphere_point());
    scattered = ray::create(rec.p, v3f_sub(target, rec.p));
    attenuation = m_albedo->value(0.0f, 0.0f, rec.p);
    return true;
}

#pragma mark - Metal

struct _metal : metal {

    _metal(const v3f &a, float f);

    bool scatter(const sptr<ray> &r_in,
                 const hit_record &rec,
                 v3f &attenuation,
                 sptr<ray> &scattered) const;

    v3f m_albedo;
    float m_fuzz;
};

_metal::_metal(const v3f &a, float f)
{
    m_albedo = a;
    m_fuzz = fminf(1.0f, f);
}

bool _metal::scatter(const sptr<ray> &r_in,
                     const hit_record &rec,
                     v3f &attenuation,
                     sptr<ray> &scattered) const
{
    v3f reflected = v3f_reflect(r_in->direction(), rec.normal);
    v3f fuzz = v3f_smul(m_fuzz, random_sphere_point());
    scattered = ray::create(rec.p, v3f_add(fuzz, reflected));
    attenuation = m_albedo;
    return v3f_dot(scattered->direction(), rec.normal) > 0.0f;
}

#pragma mark - Dieletric

struct _dielectric : dielectric {

    _dielectric(float ri) : m_ref_idx(ri) { }

    bool scatter(const sptr<ray> &r_in,
                 const hit_record &rec,
                 v3f &attenuation,
                 sptr<ray> &scattered) const;

    float m_ref_idx;
};

bool _dielectric::scatter(const sptr<ray> &r_in,
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
    if (v3f_dot(r_in->direction(), rec.normal) > 0.0f) {
        norm_out = v3f_smul(-1.0f, rec.normal);
        ni_over_nt = m_ref_idx;
        cosine = m_ref_idx * v3f_dot(rdir, rec.normal) / v3f_norm(rdir);
    } else {
        norm_out = rec.normal;
        ni_over_nt = 1.0f / m_ref_idx;
        cosine = - v3f_dot(rdir, rec.normal) / v3f_norm(rdir);
    }
    if (refract(r_in->direction(), norm_out, ni_over_nt, refracted)) {
        p_reflected = schlick(cosine, m_ref_idx);
    } else {
        p_reflected = 1.0f;
    }
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    if (dist(__prng) < p_reflected) {
        v3f reflected = v3f_reflect(rdir, rec.normal);
        scattered = ray::create(rec.p, reflected);
    } else {
        scattered = ray::create(rec.p, refracted);
    }
    return true;
}

#pragma mark - Static constructors

sptr<lambertian> lambertian::create(const sptr<Texture> &tex)
{
    return std::make_shared<_lambertian>(tex);
}

sptr<metal> metal::create(const v3f &a, float f)
{
    return std::make_shared<_metal>(a, f);
}

sptr<dielectric> dielectric::create(float ri)
{
    return std::make_shared<_dielectric>(ri);
}
