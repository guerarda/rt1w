#pragma once

#include <vector>

#include "sptr.hpp"

struct Camera;
struct Light;
struct Params;
struct Primitive;

struct RenderDescription : Object {
    static sptr<RenderDescription> create(const std::vector<sptr<Primitive>> &primitives,
                                          const std::vector<sptr<Light>> &lights,
                                          const sptr<Camera> &camera,
                                          const sptr<Params> &options);
    static sptr<RenderDescription> load(const std::string &path);

    virtual const std::vector<sptr<Primitive>> &primitives() const = 0;
    virtual const std::vector<sptr<Light>> &lights() const = 0;
    virtual sptr<Camera> camera() const = 0;
    virtual sptr<const Params> options() const = 0;
};

struct Scene : Object {
    static sptr<Scene> create(const sptr<Primitive> &world,
                              const std::vector<sptr<Light>> &lights);

    virtual sptr<Primitive> world() const = 0;
    virtual const std::vector<sptr<Light>> &lights() const = 0;
};
