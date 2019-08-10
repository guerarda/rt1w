#pragma once

#include "rt1w/geometry.hpp"
#include "rt1w/sptr.hpp"

#include <vector>

template <typename T>
struct Batch;
struct Camera;
struct Interaction;
struct Light;
struct MeshData;
struct Params;
struct Primitive;
struct Ray;

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

    virtual bounds3f bounds() const = 0;
    virtual const std::vector<sptr<Light>> &lights() const = 0;

    virtual bool intersect(const Ray &r, Interaction &isect) const = 0;
    virtual bool qIntersect(const Ray &r) const = 0;

    virtual sptr<Batch<Interaction>> intersect(const std::vector<Ray> &rays) const = 0;
    virtual sptr<Batch<Interaction>> qIntersect(const std::vector<Ray> &rays) const = 0;
};

struct PolygonScene : Scene {
    static sptr<PolygonScene> create(const sptr<MeshData> &md,
                                     const std::vector<sptr<Primitive>> &primitives,
                                     const std::vector<sptr<Light>> &lights);
};
