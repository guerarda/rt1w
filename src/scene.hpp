#pragma once

#include "sptr.hpp"

struct Camera;
struct Params;
struct Primitive;

struct Scene : Object {

    static sptr<Scene> create(const std::vector<sptr<Primitive>> &primitives,
                              const sptr<Camera> &camera,
                              const sptr<Params> &options);
    static sptr<Scene> load(const std::string &path);

    virtual std::vector<sptr<Primitive>> primitives() const = 0;
    virtual sptr<Camera> camera() const = 0;
    virtual sptr<const Params> options() const = 0;
};
