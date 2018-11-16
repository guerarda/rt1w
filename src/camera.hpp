#pragma once

#include "geometry.hpp"
#include "sptr.hpp"
#include "transform.hpp"

struct Params;
struct Ray;

struct CameraSample {
    v2f pFilm;
    v2f pLens;
};

struct Camera : Object {
    static sptr<Camera> create(const Transform &cameraToWorld,
                               const Transform &projection,
                               const rectf &bounds,
                               v2u resolution,
                               float aperture,
                               float focusDistance);

    static sptr<Camera> create(const sptr<Params> &p);

    virtual v2u resolution() const = 0;
    virtual sptr<Ray> generateRay(const CameraSample &cs) const = 0;
};

struct PerspectiveCamera : Camera {
    static sptr<Camera> create(const v3f &pos,
                               const v3f &look,
                               const v3f &up,
                               const v2u &resolution,
                               float fov,
                               float aperture,
                               float focusDistance,
                               float zNear,
                               float zFar);
};

struct OrthographicCamera : Camera {
    static sptr<Camera> create(const v3f &pos,
                               const v3f &look,
                               const v3f &up,
                               const v2u &resolution,
                               float aperture,
                               float focusDistance,
                               float znear,
                               float zFar);
};
