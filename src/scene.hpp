#pragma once

#include "sptr.hpp"

struct Params;
struct Primitive;
struct Camera;

struct Scene : Object {

    static sptr<Scene> create_json(const std::string &path);

    virtual sptr<Primitive>    primitive() const = 0;
    virtual sptr<Camera>       camera() const = 0;
    virtual sptr<const Params> options() const = 0;
};
