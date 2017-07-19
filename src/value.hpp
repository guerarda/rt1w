#pragma once

#include "sptr.hpp"
#include "types.h"

typedef buffer_type_t vtype_t;


struct Value : Object {
    static sptr<Value> create(vtype_t t, void *v, size_t count);

    virtual vtype_t type() const = 0;
    virtual size_t count() const = 0;
    virtual void value(vtype_t type, void *v, size_t off, size_t len) const = 0;
};
