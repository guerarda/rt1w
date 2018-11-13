#pragma once

#include "sptr.hpp"
#include "types.h"

struct Camera;
struct Event;
struct Integrator;
struct Scene;

struct RenderingContext : Object {
    static sptr<RenderingContext> create(const sptr<Scene> &world,
                                         const sptr<Camera> &camera,
                                         const sptr<Integrator> &integrator);

    virtual sptr<Event> schedule() = 0;
    virtual buffer_t buffer() = 0;
};
