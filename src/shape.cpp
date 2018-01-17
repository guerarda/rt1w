#include "shape.hpp"

#include "params.hpp"
#include "sphere.hpp"

#include <assert.h>

sptr<Shape> Shape::create(const sptr<Params> &p)
{
    std::string type = p->string("type");
    assert(!type.empty());

    if (type == "sphere") {
        return Sphere::create(p);
    }

    // LOG
    return nullptr;
}
