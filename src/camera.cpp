#include "camera.hpp"
#include <math.h>
#include <random>

static v3f random_in_unit_disk()
{
    static std::random_device rd;
    static std::mt19937 mt(rd());
    static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    v3f p = { 0.0f, 0.0f, 0.0f };

    do {
        p.x = dist(mt);
        p.y = dist(mt);
    } while (Dot(p, p) >= 1.0f);
    return p;
}

struct _Camera : Camera {
    _Camera(const v3f &eye,
            const v3f &lookat,
            const v3f &up,
            float vfov,
            float aspect,
            float aperture,
            float focus_dist);

    sptr<ray> make_ray(float u, float v) const;

    v3f m_bl;
    v3f m_h;
    v3f m_v;
    v3f m_org;
    float m_lens_radius;
};

_Camera::_Camera(const v3f &eye,
                 const v3f &lookat,
                 const v3f &up,
                 float vfov,
                 float aspect,
                 float aperture,
                 float focus_dist)
{
    v3f u, v, w;
    float theta = vfov * float(M_PI) / 180.0f;
    float half_h = tanf(theta/ 2.0f);
    float half_w = aspect * half_h;

    m_lens_radius = aperture;
    m_org = eye;
    w = (m_org - lookat).normalized();
    u = Cross(up, w).normalized();
    v = Cross(w, u);

    m_bl = m_org;
    m_bl -= half_w * focus_dist * u;
    m_bl -= half_h * focus_dist * v;
    m_bl -= focus_dist * w;

    m_h = 2.0f * half_w * focus_dist * u;
    m_v = 2.0f * half_h * focus_dist * v;
}

sptr<ray> _Camera::make_ray(float u, float v) const
{
    v3f rd = m_lens_radius * random_in_unit_disk();
    v3f offset = v3f{ u, v, 0.0f } * rd;
    v3f org = m_org + offset;
    v3f dir = m_bl + u * m_h + v * m_v - org;
    return ray::create(org, dir);
}

#pragma mark - Static constructors

sptr<Camera> Camera::create(const v3f &eye,
                            const v3f &lookat,
                            const v3f &up,
                            float vfov,
                            float aspect,
                            float aperture,
                            float focus_dist)
{
    return std::make_shared<_Camera>(eye, lookat, up, vfov, aspect,
                                     aperture, focus_dist);
}
