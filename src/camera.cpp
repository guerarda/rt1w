#include "camera.hpp"

#include <math.h>

#include "params.hpp"
#include "ray.hpp"
#include "value.hpp"

static v2f UniformSampleDisk(const v2f &p)
{
    double r = std::sqrt(p.x);
    double a = 2 * M_PI * p.y;

    return { (float)(r * std::cos(a)), (float)(r * std::sin(a)) };
}

struct _ProjectiveCamera : Camera
{
    _ProjectiveCamera(const Transform &cameraToWorld,
                      const Transform &projection,
                      const rectf &window,
                      v2u resolution,
                      float lensRadius,
                      float focusDist);

    sptr<ray> generateRay(const CameraSample &cs) const override;
    v2u resolution() const override { return m_resolution; }

    Transform m_cameraToWorld;
    Transform m_cameraToScreen; // Projection
    Transform m_rasterToCamera;

    rectf m_window;
    v2u   m_resolution;

    float m_lensRadius;
    float m_focusDistance;
};

_ProjectiveCamera::_ProjectiveCamera(const Transform &cameraToWorld,
                                     const Transform &projection,
                                     const rectf &window,
                                     v2u resolution,
                                     float lensRadius,
                                     float focusDist) :
    m_cameraToWorld(cameraToWorld), m_cameraToScreen(projection), m_window(window),
    m_resolution(resolution), m_lensRadius(lensRadius), m_focusDistance(focusDist)
{
    Transform screenToRaster = Transform::Scale(resolution.x, resolution.y, 1.0f) *
        Transform::Scale(1.0f / window.size.x,
                         -1.0f / window.size.y,
                         1.0f) *
        Transform::Translate({ -window.org.x, window.org.y, 0.0f });
    m_rasterToCamera = Inverse(screenToRaster * m_cameraToScreen);
}

sptr<ray> _ProjectiveCamera::generateRay(const CameraSample &cs) const
{
    v3f pCamera = Mulp(m_rasterToCamera, v3f{ cs.pFilm.x, cs.pFilm.y, 0.0f });

    v3f org = { 0.0f, 0.0f, 0.0f };
    v3f dir = Normalize(pCamera);
    sptr<ray> r = ray::create(org, dir);

    if (m_lensRadius > 0.0f) {
        v2f pLens = m_lensRadius * UniformSampleDisk(cs.pLens);
        float t = m_focusDistance / std::abs(dir.z);
        v3f pFocus = r->point(t);

        org = { pLens.x, pLens.y, 0.0f };
        dir = Normalize(pFocus - org);

        r = ray::create(org, dir);
    }
    return m_cameraToWorld(r);
}

#pragma mark - Static constructors

sptr<Camera> Camera::create(const sptr<Params> &p)
{
    std::string type = Params::string(p, "type", "perspective");
    if (type == "custom") {
        m44f pos = Params::matrix4x4f(p, "cameratoworld", m44f_identity());
        m44f proj = Params::matrix4x4f(p, "projection", m44f_identity());
        v2u res = Params::vector2u(p, "resolution", { 1920, 1080 });
        float aperture = Params::f32(p, "aperture", 0.1f);
        float fdist = Params::f32(p, "focusdistance", 1.0f);

        rectf bounds;
        if (sptr<Value> vb = p->value("bounds")) {
            vb->value(TYPE_FLOAT32, &bounds.org.x, 0, 4);
        } else {
            float ar = (float)res.x / res.y;
            bounds = { { -ar, -1.0f }, v2f{ 2.0f * ar, 2.0f } };
        }
        return Camera::create(Transform(pos), Transform(proj), bounds, res, aperture, fdist);
    }
    if (type == "orthographic") {
        v3f pos = Params::vector3f(p, "position", { 1.0f, 1.0f, 1.0f });
        v3f look = Params::vector3f(p, "lookat", { 0.0f, 0.0f, 0.0f });
        v3f up = Params::vector3f(p, "up", { 0.0f, 1.0f, 0.0f });
        v2u res = Params::vector2u(p, "resolution", { 1920, 1080 });
        float aperture = Params::f32(p, "aperture", 0.1f);
        float fdist = Params::f32(p, "focusdistance", 1.0f);
        float znear = Params::f32(p, "znear", -0.1f);
        float zfar = Params::f32(p, "zfar", 1000.0f);

        return OrthographicCamera::create(pos, look, up, res, aperture, fdist, znear, zfar);
    }
    v3f pos = Params::vector3f(p, "position", { 1.0f, 1.0f, 1.0f });
    v3f look = Params::vector3f(p, "lookat", { 0.0f, 0.0f, 0.0f });
    v3f up = Params::vector3f(p, "up", { 0.0f, 1.0f, 0.0f });
    v2u res = Params::vector2u(p, "resolution", { 1920, 1080 });
    float fov = Params::f32(p, "fov", 60.0f);
    float aperture = Params::f32(p, "aperture", 0.1f);
    float fdist = Params::f32(p, "focusdistance", 1.0f);
    float znear = Params::f32(p, "znear", -0.1f);
    float zfar = Params::f32(p, "zfar", 1000.0f);

    return PerspectiveCamera::create(pos, look, up, res, fov, aperture, fdist, znear, zfar);
}

sptr<Camera> Camera::create(const Transform &cameraToWorld,
                            const Transform &projection,
                            const rectf &bounds,
                            v2u resolution,
                            float aperture,
                            float focusDistance)
{
    return std::make_shared<_ProjectiveCamera>(cameraToWorld,
                                               projection,
                                               bounds,
                                               resolution,
                                               aperture,
                                               focusDistance);
}

sptr<Camera> PerspectiveCamera::create(const v3f &pos,
                               const v3f &look,
                               const v3f &up,
                               const v2u &resolution,
                               float fov,
                               float aperture,
                               float focusDistance,
                               float zNear,
                               float zFar)
{
    Transform lookat = Transform::LookAt(pos, look, up);
    Transform proj = Transform::Perspective(fov, zNear, zFar);

    float ar = (float)resolution.x / resolution.y;
    rectf screen = { { -ar, -1.0f }, { 2.0f * ar, 2.0f } };

    return std::make_shared<_ProjectiveCamera>(lookat,
                                               proj,
                                               screen,
                                               resolution,
                                               aperture,
                                               focusDistance);
}

sptr<Camera> OrthographicCamera::create(const v3f &pos,
                                        const v3f &look,
                                        const v3f &up,
                                        const v2u &resolution,
                                        float aperture,
                                        float focusDistance,
                                        float zNear,
                                        float zFar)
{
    Transform lookat = Transform::LookAt(pos, look, up);
    Transform proj = Transform::Orthographic(zNear, zFar);

    float ar = (float)resolution.x / resolution.y;
    rectf screen = { { -ar, -1.0f }, { 2.0f * ar, 2.0f } };

    return std::make_shared<_ProjectiveCamera>(lookat,
                                               proj,
                                               screen,
                                               resolution,
                                               aperture,
                                               focusDistance);
}
