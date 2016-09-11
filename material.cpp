#include "material.hpp"
#include <random>

#pragma mark - Lambertian

v3f random_sphere_point()
{
    static std::random_device rd;
    static std::mt19937 mt(rd());
    static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    v3f p;

    do {
        p.x = dist(mt);
        p.y = dist(mt);
        p.z = dist(mt);
    } while (v3f_norm_sq(p) >= 1.0f);

    return p;
}

struct _lambertian : lambertian {

    _lambertian(const v3f &a) : m_albedo(a) { }
    virtual ~_lambertian() { }

    bool scatter(const sptr<ray> &r_in,
                 const hit_record &rec,
                 v3f &attenuation,
                 sptr<ray> &scattered) const;

    v3f m_albedo;
};

bool _lambertian::scatter(__unused const sptr<ray> &r_in,
                          const hit_record &rec,
                          v3f &attenuation,
                          sptr<ray> &scattered) const
{
    v3f target = v3f_add(v3f_add(rec.p, v3f_normalize(rec.normal)),
                         random_sphere_point());
    scattered = ray::create(rec.p, v3f_sub(target, rec.p));
    attenuation = m_albedo;
    return true;
}

#pragma mark - Metal

struct _metal : metal {

    _metal(const v3f &a, float f);
    virtual ~_metal() { }

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
    v3f reflected = v3f_reflect(r_in->direction(), v3f_normalize(rec.normal));
    v3f fuzz = v3f_smul(m_fuzz, random_sphere_point());
    scattered = ray::create(rec.p, v3f_add(fuzz, reflected));
    attenuation = m_albedo;
    return v3f_dot(scattered->direction(), rec.normal) > 0.0f;
}

#pragma mark - Static constructors

sptr<lambertian> lambertian::create(const v3f &a)
{
    return std::make_shared<_lambertian>(a);
}

sptr<metal> metal::create(const v3f &a, float f)
{
    return std::make_shared<_metal>(a, f);
}
