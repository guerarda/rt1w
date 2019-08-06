#pragma once

#include "rt1w/sptr.hpp"

struct Arena : Object {
    static uptr<Arena> create();

    virtual void *alloc(size_t n) = 0;
};
