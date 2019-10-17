#include "rt1w/shape.hpp"

#include "shapes/mesh.hpp"
#include "shapes/sphere.hpp"

#include "rt1w/error.h"
#include "rt1w/params.hpp"

sptr<Shape> Shape::create(const sptr<Params> &p)
{
    std::string type = p->string("type");

    if (type.empty()) {
        ERROR("Unspecified shape type");
        return nullptr;
    }
    if (type == "sphere") {
        return Sphere::create(p);
    }
    if (type == "mesh") {
        return Mesh::create(p);
    }
    ERROR("Unknown shape : \"%s\"", type.c_str());

    return nullptr;
}
