#include "camera.hpp"
#include <math.h>
#include <random>

#include "ray.hpp"
#include "params.hpp"
#include "value.hpp"

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
            v2u resolution,
            float fov,
            float aperture,
            float focus_dist);

    v2u       resolution() const { return m_resolution; }
    sptr<ray> make_ray(float u, float v) const;

    v2u m_resolution;
    v3f m_bl;
    v3f m_h;
    v3f m_v;
    v3f m_org;
    float m_lens_radius;
};

_Camera::_Camera(const v3f &eye,
                 const v3f &lookat,
                 const v3f &up,
                 v2u res,
                 float fov,
                 float aperture,
                 float focus_dist)
{
    v3f u, v, w;
    float aspect = (float)res.x / res.y;
    float theta = fov * float(M_PI) / 180.0f;
    float half_h = tanf(theta/ 2.0f);
    float half_w = aspect * half_h;

    m_resolution = res;
    m_lens_radius = aperture;
    m_org = eye;
    w = Normalize(m_org - lookat);
    u = Normalize(Cross(up, w));
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
                            v2u res,
                            float fov,
                            float aperture,
                            float focus_dist)
{
    return std::make_shared<_Camera>(eye, lookat, up, res,
                                     fov, aperture, focus_dist);
}

sptr<Camera> Camera::create(const sptr<Params> &p)
{
    sptr<Value> pos = p->value("position");
    if (!pos) {
        // LOG
    }
    sptr<Value> lookat = p->value("lookat");
    if (!lookat) {
        // LOG
    }
    sptr<Value> up = p->value("up");
    if (!up) {
        // LOG
    }
    sptr<Value> res = p->value("resolution");
    if (!res) {
        // LOG
    }
    sptr<Value> fov = p->value("fov");
    if (!fov) {
        // LOG
    }
    sptr<Value> aperture = p->value("aperture");
    if (!aperture) {
        //LOG
    }
    sptr<Value> fdist = p->value("focusdistance");
    if (!fdist) {
        // LOG
    }

    return Camera::create(pos->vector3f(),
                          lookat->vector3f(),
                          up->vector3f(),
                          res->vector2u(),
                          fov->f32(),
                          aperture->f32(),
                          fdist->f32());
}
