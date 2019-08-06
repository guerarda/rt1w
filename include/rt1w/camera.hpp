#pragma once

#include "rt1w/geometry.hpp"
#include "rt1w/sptr.hpp"

struct Params;
struct Ray;
struct Transform;

struct CameraSample {
    v2f pFilm;
    v2f pLens;
};

struct Camera : Object {
    static sptr<Camera> create(const sptr<Params> &p);

    virtual v3f position() const = 0;
    virtual v2u resolution() const = 0;
    virtual Ray generateRay(const CameraSample &cs) const = 0;
};

struct PerspectiveCamera : Camera {
    static sptr<Camera> create(const v3f &pos,
                               const v3f &look,
                               const v3f &up,
                               const v2u &resolution,
                               const v2f &screen,
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
                               const v2f &screen,
                               float aperture,
                               float focusDistance,
                               float zNear,
                               float zFar);
};
