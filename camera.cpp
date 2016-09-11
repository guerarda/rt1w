#include "camera.hpp"

struct _camera : camera {
    _camera(const v3f &bl,
            const v3f &h,
            const v3f &v,
            const v3f &org) : m_bl(bl), m_h(h), m_v(v), m_org(org) { }
    virtual ~_camera() { }

    sptr<ray> make_ray(float u, float v) const;

    v3f m_bl;
    v3f m_h;
    v3f m_v;
    v3f m_org;
};

sptr<ray> _camera::make_ray(float u, float v) const
{
    v3f dir = v3f_add(v3f_add(m_bl, v3f_smul(u, m_h)),
                      v3f_smul(v, m_v));
    return ray::create(m_org, dir);
}

#pragma mark - Static constructors

sptr<camera> camera::create(const v3f &bottom_left,
                            const v3f &h,
                            const v3f &v,
                            const v3f &org)
{
    return std::make_shared<_camera>(bottom_left, h, v, org);
}
