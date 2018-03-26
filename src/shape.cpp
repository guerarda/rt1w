#include "shape.hpp"

#include "params.hpp"
#include "sphere.hpp"
#include "mesh.hpp"
#include "error.h"

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
