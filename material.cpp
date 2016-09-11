#include "material.hpp"
#include <random>

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
    } while (v3f_norm_sq(p) > 1.0f);

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
    v3f target = v3f_add(v3f_add(rec.p, rec.normal),
                         random_sphere_point());
    scattered = ray::create(rec.p, v3f_sub(target, rec.p));
    attenuation = m_albedo;
    return true;
}

#pragma mark - Static constructors

sptr<lambertian> lambertian::create(const v3f &a)
{
    return std::make_shared<_lambertian>(a);
}
