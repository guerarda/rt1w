#include "camera.hpp"

#include "params.hpp"
#include "ray.hpp"
#include "sampling.hpp"
#include "transform.hpp"
#include "value.hpp"

enum struct ProjectionType { Perspective, Orthographic };

struct _ProjectiveCamera : Camera {
    _ProjectiveCamera(const Transform &cameraToWorld,
                      const Transform &projection,
                      const rectf &window,
                      v2u resolution,
                      float lensRadius,
                      float focusDist,
                      ProjectionType type);

    sptr<Ray> generateRay(const CameraSample &cs) const override;
    v2u resolution() const override { return m_resolution; }

    Transform m_cameraToWorld;
    Transform m_cameraToScreen;    // Projection
    Transform m_rasterToCamera;

    rectf m_window;
    v2u m_resolution;

    float m_lensRadius;
    float m_focusDistance;

    ProjectionType m_type;
};

_ProjectiveCamera::_ProjectiveCamera(const Transform &cameraToWorld,
                                     const Transform &projection,
                                     const rectf &window,
                                     v2u resolution,
                                     float lensRadius,
                                     float focusDist,
                                     ProjectionType type) :
    m_cameraToWorld(cameraToWorld),
    m_cameraToScreen(projection),
    m_window(window),
    m_resolution(resolution),
    m_lensRadius(lensRadius),
    m_focusDistance(focusDist),
    m_type(type)
{
    Transform screenToRaster =
        Transform::Scale(resolution.x, resolution.y, 1.0f)
        * Transform::Scale(1.0f / window.size.x, -1.0f / window.size.y, 1.0f)
        * Transform::Translate({ -window.org.x, window.org.y, 0.0f });
    m_rasterToCamera = Inverse(screenToRaster * m_cameraToScreen);
}

sptr<Ray> _ProjectiveCamera::generateRay(const CameraSample &cs) const
{
    v3f pCamera = Mulp(m_rasterToCamera, v3f{ cs.pFilm.x, cs.pFilm.y, 0.0f });

    v3f org = m_type == ProjectionType::Perspective ? v3f{ 0.0f, 0.0f, 0.0f } : pCamera;
    v3f dir = m_type == ProjectionType::Perspective ? Normalize(pCamera)
                                                    : v3f{ 0.0f, 0.0f, -1.0f };
    sptr<Ray> r = Ray::create(org, dir);

    if (m_lensRadius > 0.0f) {
        v2f pLens = m_lensRadius * UniformSampleDisk(cs.pLens);
        float t = m_focusDistance / std::abs(dir.z);
        v3f pFocus = r->point(t);

        org = { pLens.x, pLens.y, 0.0f };
        dir = Normalize(pFocus - org);

        r = Ray::create(org, dir);
    }
    return m_cameraToWorld(r);
}

#pragma mark - Static constructors

sptr<Camera> Camera::create(const sptr<Params> &p)
{
    std::string type = Params::string(p, "type", "perspective");

    if (type == "perspective") {
        v3f pos = Params::vector3f(p, "position", { 1.0f, 1.0f, 1.0f });
        v3f look = Params::vector3f(p, "lookat", { 0.0f, 0.0f, 0.0f });
        v3f up = Params::vector3f(p, "up", { 0.0f, 1.0f, 0.0f });
        v2u res = Params::vector2u(p, "resolution", { 1920, 1080 });
        float fov = Params::f32(p, "fov", 60.0f);
        float aperture = Params::f32(p, "aperture", 0.1f);
        float fdist = Params::f32(p, "focusdistance", 1.0f);
        float znear = Params::f32(p, "znear", -0.1f);
        float zfar = Params::f32(p, "zfar", -1000.0f);

        float ar = (float)res.x / res.y;
        v2f screen = Params::vector2f(p, "screen", { 2.0f * ar, 2.0f });

        return PerspectiveCamera::create(pos,
                                         look,
                                         up,
                                         res,
                                         screen,
                                         fov,
                                         aperture,
                                         fdist,
                                         znear,
                                         zfar);
    }
    if (type == "orthographic") {
        v3f pos = Params::vector3f(p, "position", { 1.0f, 1.0f, 1.0f });
        v3f look = Params::vector3f(p, "lookat", { 0.0f, 0.0f, 0.0f });
        v3f up = Params::vector3f(p, "up", { 0.0f, 1.0f, 0.0f });
        v2u res = Params::vector2u(p, "resolution", { 1920, 1080 });
        float aperture = Params::f32(p, "aperture", 0.1f);
        float fdist = Params::f32(p, "focusdistance", 1.0f);
        float znear = Params::f32(p, "znear", -0.1f);
        float zfar = Params::f32(p, "zfar", -1000.0f);

        float ar = (float)res.x / res.y;
        v2f screen = Params::vector2f(p, "screen", { 2.0f * ar, 2.0f });

        return OrthographicCamera::create(pos,
                                          look,
                                          up,
                                          res,
                                          screen,
                                          aperture,
                                          fdist,
                                          znear,
                                          zfar);
    }
    error("Unknown camera : \"%s\"", type.c_str());
    return nullptr;
}

sptr<Camera> PerspectiveCamera::create(const v3f &pos,
                                       const v3f &look,
                                       const v3f &up,
                                       const v2u &resolution,
                                       const v2f &screen,
                                       float fov,
                                       float aperture,
                                       float focusDistance,
                                       float zNear,
                                       float zFar)
{
    Transform lookat = Transform::LookAt(pos, look, up);
    Transform proj = Transform::Perspective(fov, zNear, zFar);
    rectf bounds = { { 0.5f * -screen.x, 0.5f * -screen.y }, screen };

    return std::make_shared<_ProjectiveCamera>(lookat,
                                               proj,
                                               bounds,
                                               resolution,
                                               aperture,
                                               focusDistance,
                                               ProjectionType::Perspective);
}

sptr<Camera> OrthographicCamera::create(const v3f &pos,
                                        const v3f &look,
                                        const v3f &up,
                                        const v2u &resolution,
                                        const v2f &screen,
                                        float aperture,
                                        float focusDistance,
                                        float zNear,
                                        float zFar)
{
    Transform lookat = Transform::LookAt(pos, look, up);
    Transform proj = Transform::Orthographic(zNear, zFar);
    rectf bounds = { { 0.5f * -screen.x, 0.5f * -screen.y }, screen };

    return std::make_shared<_ProjectiveCamera>(lookat,
                                               proj,
                                               bounds,
                                               resolution,
                                               aperture,
                                               focusDistance,
                                               ProjectionType::Orthographic);
}
