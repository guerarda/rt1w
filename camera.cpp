#include "camera.hpp"
#include <math.h>

struct _camera : camera {
    _camera(const v3f &eye,
            const v3f &lookat,
            const v3f &up,
            float vfov,
            float aspect);
    virtual ~_camera() { }

    sptr<ray> make_ray(float u, float v) const;

    v3f m_bl;
    v3f m_h;
    v3f m_v;
    v3f m_org;
};

_camera::_camera(const v3f &eye,
                 const v3f &lookat,
                 const v3f &up,
                 float vfov,
                 float aspect)
{
    v3f u, v, w;
    float theta = vfov * M_PI / 180.0f;
    float half_h = tanf(theta/ 2.0f);
    float half_w = aspect * half_h;

    m_org = eye;
    w = v3f_normalize(v3f_sub(m_org, lookat));
    u = v3f_normalize(v3f_cross(up, w));
    v = v3f_cross(w, u);

    m_bl = v3f_sub(m_org, v3f_add(v3f_add(v3f_smul(half_w, u),
                                          v3f_smul(half_h, v)),
                                  w));
    m_h = v3f_smul(2.0f * half_w, u);
    m_v = v3f_smul(2.0f * half_h, v);
}

sptr<ray> _camera::make_ray(float u, float v) const
{
    v3f dir = v3f_sub(v3f_add(v3f_add(m_bl, v3f_smul(u, m_h)),
                              v3f_smul(v, m_v)),
                      m_org);
    return ray::create(m_org, dir);
}

#pragma mark - Static constructors

sptr<camera> camera::create(const v3f &eye,
                            const v3f &lookat,
                            const v3f &up,
                            float vfov,
                            float aspect)
{
    return std::make_shared<_camera>(eye, lookat, up, vfov, aspect);
}
