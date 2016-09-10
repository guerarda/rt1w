#include "ray.hpp"

struct _ray : ray {

    _ray(const v3f &org, const v3f &dir) : m_org(org), m_dir(dir) { }
    virtual ~_ray() { }

    v3f origin() const { return m_org; }
    v3f direction() const { return m_dir; }
    v3f point_at_param(float t) const;

    v3f m_org;
    v3f m_dir;
};

v3f _ray::point_at_param(float t) const
{
    return v3f_add(m_org, v3f_smul(t, m_dir));
}

#pragma mark - Static constructors

sptr<ray> ray::create(const v3f &org, const v3f &dir)
{
    return std::make_shared<_ray>(org, dir);
}
