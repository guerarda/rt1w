#include "ray.hpp"

struct _Ray : Ray {

    _Ray(const v3f &org, const v3f &dir) : m_org(org), m_dir(dir) { }

    v3f origin() const { return m_org; }
    v3f direction() const { return m_dir; }
    v3f point(float t) const;

    v3f m_org;
    v3f m_dir;
};

v3f _Ray::point(float t) const
{
    return m_org + t * m_dir;
}

#pragma mark - Static constructors

sptr<Ray> Ray::create(const v3f &org, const v3f &dir)
{
    return std::make_shared<_Ray>(org, dir);
}
