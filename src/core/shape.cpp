#include "rt1w/shape.hpp"

#include "rt1w/error.h"
#include "rt1w/mesh.hpp"
#include "rt1w/params.hpp"
#include "rt1w/sphere.hpp"

sptr<Shape> Shape::create(const sptr<Params> &p)
{
    std::string type = p->string("type");

    if (type.empty()) {
        error("Unspecified shape type");
        return nullptr;
    }
    if (type == "sphere") {
        return Sphere::create(p);
    }
    if (type == "mesh") {
        return Mesh::create(p);
    }
    error("Unknown shape : \"%s\"", type.c_str());

    return nullptr;
}
