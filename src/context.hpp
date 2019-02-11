#pragma once

#include "sptr.hpp"

struct Camera;
struct Image;
struct Integrator;
struct Scene;

struct Render : Object {
    static sptr<Render> create(const sptr<Scene> &scene,
                               const sptr<Camera> &camera,
                               const sptr<Integrator> &integrator);

    virtual sptr<Image> image() const = 0;
};
