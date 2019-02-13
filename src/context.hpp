#pragma once

#include "sptr.hpp"
#include "types.h"

struct Camera;
struct Image;
struct Integrator;
struct Scene;

/* order must be RGB, type can be uint8, uint16 or float32 */
struct RenderOptions {
    buffer_type_t type;
    buffer_order_t order;
};

struct Render : Object {
    static sptr<Render> create(const sptr<Scene> &scene,
                               const sptr<Camera> &camera,
                               const sptr<Integrator> &integrator,
                               const RenderOptions &options);

    virtual sptr<Image> image() const = 0;
};
